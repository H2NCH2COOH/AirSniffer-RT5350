#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=as-spi-gpio-dev
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/as-spi-gpio-dev
  SUBMENU:=SPI Support
  TITLE:=AirSniffer GPIO-based SPI device
  DEPENDS:=@GPIO_SUPPORT +kmod-spi-bitbang +kmod-spi-dev +kmod-as-spi-gpio
  FILES:=$(PKG_BUILD_DIR)/as-spi-gpio-dev.ko
  KCONFIG:=
endef

define KernelPackage/as-spi-gpio-dev/description
 AirSniffer GPIO-based SPI device
endef

EXTRA_KCONFIG:= \
	CONFIG_AS_SPI_GPIO_DEV=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,as-spi-gpio-dev))
