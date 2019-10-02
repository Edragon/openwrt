include $(TOPDIR)/rules.mk

PKG_NAME:=gpio_control_app
PKG_VERSION:=1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/gpio_control_app
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Control gpios
endef

define Package/gpio_control_app/description
	Work with gpio_control_driver to control GPIOs
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/gpio_control_app/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpio_control_app $(1)/bin/
endef

$(eval $(call BuildPackage,gpio_control_app))
