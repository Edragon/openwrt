include $(TOPDIR)/rules.mk

PKG_NAME:=hello_world
PKG_VERSION:=1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/hello_world
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=Hello world -prints a hello world message
endef

define Package/hello_world/description
	If you can't figure out what this program does, you're probably  
	brain-dead and need immediate medical attention.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/hello_world/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hello_world $(1)/bin/
endef

$(eval $(call BuildPackage,hello_world))
