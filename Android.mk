#Android makefile to build kernel as a part of Android Build
PERL		= perl


KERNEL_DEFCONFIG := msm8226_defconfig


ifeq ($(TARGET_PREBUILT_KERNEL),)

KERNEL_OUT := ./KERNEL_OBJ
KERNEL_CONFIG := $(KERNEL_OUT)/.config
ifeq ($(TARGET_KERNEL_APPEND_DTB), true)
TARGET_PREBUILT_INT_KERNEL := $(KERNEL_OUT)/arch/arm/boot/zImage-dtb
else
TARGET_PREBUILT_INT_KERNEL := $(KERNEL_OUT)/arch/arm/boot/zImage
endif
KERNEL_HEADERS_INSTALL := $(KERNEL_OUT)/usr
KERNEL_MODULES_INSTALL := $(KERNEL_OUT)/system
KERNEL_MODULES_OUT := $(KERNEL_OUT)/modules
KERNEL_IMG=$(KERNEL_OUT)/arch/arm/boot/Image

DTS_NAMES ?= $(shell $(PERL) -e 'while (<>) {$$a = $$1 if /CONFIG_ARCH_((?:MSM|QSD|MPQ)[a-zA-Z0-9]+)=y/; $$r = $$1 if /CONFIG_MSM_SOC_REV_(?!NONE)(\w+)=y/; $$arch = $$arch.lc("$$a$$r ") if /CONFIG_ARCH_((?:MSM|QSD|MPQ)[a-zA-Z0-9]+)=y/} print $$arch;' $(KERNEL_CONFIG))
KERNEL_USE_OF ?= $(shell $(PERL) -e '$$of = "n"; while (<>) { if (/CONFIG_USE_OF=y/) { $$of = "y"; break; } } print $$of;' ./arch/arm/configs/$(KERNEL_DEFCONFIG))

ifeq "$(KERNEL_USE_OF)" "y"
DTS_FILES = $(wildcard ./arch/arm/boot/dts/yep_product_*.dts)
DTS_FILE = $(lastword $(subst /, ,$(1)))
DTB_FILE = $(addprefix $(KERNEL_OUT)/arch/arm/boot/,$(patsubst %.dts,%.dtb,$(call DTS_FILE,$(1))))
ZIMG_FILE = $(addprefix $(KERNEL_OUT)/arch/arm/boot/,$(patsubst %.dts,%-zImage,$(call DTS_FILE,$(1))))
KERNEL_ZIMG = $(KERNEL_OUT)/arch/arm/boot/zImage
DTC = $(KERNEL_OUT)/scripts/dtc/dtc

define append-dtb
mkdir -p $(KERNEL_OUT)/arch/arm/boot;\
$(foreach DTS_NAME, $(DTS_NAMES), \
   $(foreach d, $(DTS_FILES), \
      $(DTC) -p 1024 -O dtb -o $(call DTB_FILE,$(d)) $(d); \
      cat $(KERNEL_ZIMG) $(call DTB_FILE,$(d)) > $(call ZIMG_FILE,$(d));))
endef
else

define append-dtb
endef
endif

ifeq ($(TARGET_USES_UNCOMPRESSED_KERNEL),true)
$(info Using uncompressed kernel)
TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/piggy
else
TARGET_PREBUILT_KERNEL := $(TARGET_PREBUILT_INT_KERNEL)
endif

define mv-modules
mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`;\
ko=`find $$mpath/kernel -type f -name *.ko`;\
for i in $$ko; do mv $$i $(KERNEL_MODULES_OUT)/; done;\
fi
endef

define clean-module-folder
mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`; rm -rf $$mpath;\
fi
endef

$(KERNEL_OUT):
	mkdir -p $(KERNEL_OUT)

$(KERNEL_CONFIG): $(KERNEL_OUT)
	$(MAKE)  O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- $(KERNEL_DEFCONFIG)

$(KERNEL_OUT)/piggy : $(TARGET_PREBUILT_INT_KERNEL)
	$(hide) gunzip -c $(KERNEL_OUT)/arch/arm/boot/compressed/piggy.gzip > $(KERNEL_OUT)/piggy

$(TARGET_PREBUILT_INT_KERNEL): $(KERNEL_OUT) $(KERNEL_CONFIG) $(KERNEL_HEADERS_INSTALL)
	$(MAKE)  O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi-
	$(MAKE)  O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- modules
	$(MAKE)  O=$(KERNEL_OUT) INSTALL_MOD_PATH=$(KERNEL_MODULES_INSTALL) INSTALL_MOD_STRIP=1 ARCH=arm CROSS_COMPILE=arm-eabi- modules_install
	$(mv-modules)
	$(clean-module-folder)
	$(append-dtb)

$(KERNEL_HEADERS_INSTALL): $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE)  O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- headers_install

kerneltags: $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE)  O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- tags

kernelconfig: $(KERNEL_OUT) $(KERNEL_CONFIG)
	env KCONFIG_NOTIMESTAMP=true \
	     $(MAKE)  O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- menuconfig
	env KCONFIG_NOTIMESTAMP=true \
	     $(MAKE)  O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- savedefconfig
	cp $(KERNEL_OUT)/defconfig arch/arm/configs/$(KERNEL_DEFCONFIG)

DTBTOOL := ./dtbTool

INSTALLED_DTIMAGE_TARGET := $(KERNEL_OUT)/dt.img

possible_dtb_dirs = $(KERNEL_OUT)/arch/arm/boot/dts/ $(KERNEL_OUT)/arch/arm/boot/
dtb_dir = $(firstword $(wildcard $(possible_dtb_dirs)))

define build-dtimage-target
    $(call pretty,"Target dt image: $(INSTALLED_DTIMAGE_TARGET)")
    $(hide) $(DTBTOOL) -o $@ -s 2048 -p $(KERNEL_OUT)/scripts/dtc/ $(dtb_dir)
    $(hide) chmod a+r $@
endef	
$(INSTALLED_DTIMAGE_TARGET): $(TARGET_PREBUILT_INT_KERNEL)
	$(build-dtimage-target)	
bootimage: $(INSTALLED_DTIMAGE_TARGET)
	./mkbootimg  --kernel $(KERNEL_OUT)/arch/arm/boot/zImage --ramdisk ./ramdisk.img \
	    --cmdline "console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 loglevel=7 androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x37" \
		--base 0x00000000 --pagesize 2048 --ramdisk_offset 0x02000000 --tags_offset 0x01E00000 --dt $(KERNEL_OUT)/dt.img  --output $(KERNEL_OUT)/boot.img
		
endif
