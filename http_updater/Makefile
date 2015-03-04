include $(TOPDIR)/rules.mk

PKG_NAME:=http-updater
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=http-updater
endef

define Package/$(PKG_NAME)/description
	Updater based on http
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/www
	$(INSTALL_BIN) ./files/update.html $(1)/www/
	$(INSTALL_BIN) ./files/update_config.html $(1)/www/
	
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) ./files/update $(1)/www/cgi-bin/
	$(INSTALL_BIN) ./files/update_config $(1)/www/cgi-bin/
	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cfg_updater $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/urldecoder $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))