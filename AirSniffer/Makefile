include $(TOPDIR)/rules.mk

PKG_NAME:=AirSniffer
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Air Sniffer
	DEPENDS:=+libugpio
endef

define Package/$(PKG_NAME)/description
	Air Sniffer main functionality package
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	##Device.id
	#$(INSTALL_DIR) $(1)/etc/config
	#$(CP) ./files/device.id $(1)/etc/config/
	
	##Device.conf
	#$(INSTALL_DIR) $(1)/etc/config
	#$(CP) ./files/device.conf $(1)/etc/config/
	
	##SSH Pubkey
	#$(INSTALL_DIR) $(1)/etc/dropbear
	#$(CP) ./files/authorized_keys $(1)/etc/dropbear/
	
	##Main
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Main/AirSniffer $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Main/Welcome $(1)/usr/bin/
	$(INSTALL_BIN) ./files/Main/read_temp.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/Main/change_mac.sh $(1)/usr/bin/
	
	#$(INSTALL_DIR) $(1)/
	#$(INSTALL_BIN) ./files/Main/dev_setup.sh $(1)/
	
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./files/Main/udhcpc.script $(1)/etc/
	
	
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/Main/as_init.sh $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/Main/welcome.sh $(1)/etc/init.d/
	
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/Main/dhcpd.conf $(1)/etc/config/
	#$(INSTALL_BIN) ./files/Main/network $(1)/etc/config/
	$(CP) ./files/device.version $(1)/etc/config
	
	##CGI
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/CGI/wifi_setup_agent $(1)/usr/bin/
	
	$(INSTALL_DIR) $(1)/www
	$(INSTALL_BIN) ./files/CGI/index.html $(1)/www/
	$(INSTALL_BIN) ./files/CGI/cali.html $(1)/www/
	
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) ./files/CGI/ask $(1)/www/cgi-bin/
	$(INSTALL_BIN) ./files/CGI/sethost $(1)/www/cgi-bin/
	$(INSTALL_BIN) ./files/CGI/calibrate $(1)/www/cgi-bin/
	$(INSTALL_BIN) ./files/CGI/do_calibration $(1)/www/cgi-bin/
	
	##imgs
	$(CP) ./files/imgs $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))