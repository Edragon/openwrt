#
# Copyright (C) 2017 <wurobinson@zhuotk.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gpio_control_driver
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gpio_control_driver
  SUBMENU:=Other modules
  DEPENDS:=@GPIO_SUPPORT
  TITLE:=Driver for JS9331/JS7628 gpios control
  FILES:=$(PKG_BUILD_DIR)/gpio_control_driver.ko
  KCONFIG:=
  AUTOLOAD:=$(call AutoLoad,30,gpio_control_driver)
endef

define KernelPackage/gpio_control_driver/description
 Kernel module to control gpios for JS9331 and JS7628
endef

EXTRA_KCONFIG:= \
        CONFIG_GPIO_CONTROL_DRIVER=m

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(BUILDFLAGS)" \
		$(EXTRA_KCONFIG)
endef

$(eval $(call KernelPackage,gpio_control_driver))
