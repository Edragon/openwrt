include $(TOPDIR)/rules.mk

PKG_NAME:=net_control_client
PKG_VERSION:=1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/net_control_client
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=net control client
	DEPENDS:=+libpthread
endef

define Package/net_control_client/description
	A sample socket net control client, write for JS7628 and JS9331, www.zhuotk.com
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/net_control_client/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/net_control_client $(1)/bin/
endef

$(eval $(call BuildPackage,net_control_client))
