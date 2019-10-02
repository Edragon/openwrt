include $(TOPDIR)/rules.mk

PKG_NAME:=net_control_server
PKG_VERSION:=1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/net_control_server
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=net control server
	DEPENDS:=+libpthread
endef

define Package/net_control_server/description
	A sample socket net control server, write for JS7628 and JS9331, www.zhuotk.com
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/net_control_server/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/net_control_server $(1)/bin/
endef

$(eval $(call BuildPackage,net_control_server))