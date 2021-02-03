#include "network.h"

//long unsigned int last_scan_millis = 0;

namespace ui {
	namespace network {
		bool sta_connected = false;
		int avail_networks = 0;
		bool scan = false;
				
		void settings::defaults() {
			(String("ap_generic") /*+ millis()*/).toCharArray(ap_ssid, 80);
			ap_pass[0] = '\0';
			ap_ip = IPAddress(7, 7, 7, 7);
			ap_gateway = IPAddress(7, 7, 7, 7);
			ap_subnet = IPAddress(255, 255, 255, 0);
			
			sta_enabled = false;
			sta_ssid[0] = '\0';
			sta_pass[0] = '\0';
			sta_ip = IPAddress(192, 168, 1, 227);
			sta_gateway = IPAddress(192, 168, 1, 1);
			sta_subnet = IPAddress(255, 255, 255, 0);
		}
			
		void settings::save()
		{
			DEBUG_MSG("Saving network settings...\n");
			File f = LittleFS.open("/network.bin", "w");
			if (f) {
				f.write((uint8_t *)this, sizeof(settings));
				f.close();
				
				committed();
				
				DEBUG_MSG("\tSave succeed!\n");
			} else {
				DEBUG_MSG("\tSave failed!\n");
			}
		}
			
		void settings::load()
		{
			DEBUG_MSG("Loading network settings...\n");
			File f = LittleFS.open("/network.bin", "r");
			if (f) {
				f.read((uint8_t *)this, sizeof(settings));
				f.close();
				
				DEBUG_MSG("\tLoad succeed!\n");
			} else {
				DEBUG_MSG("\tLoad failed!\n");
				defaults();
			}
		}
		
		struct settings &settings::changed()
		{
			settings::need_commit = true;
			return *this;
		}

		void settings::committed()
		{
			settings::need_commit = false;
		}
		
		settings::settings()
		{
			defaults();
		}
		
		bool settings::need_commit = false;
		struct settings settings;
		
		bool passphrase_is_valid(String val)
		{
			if (val.length() && (val.length() > 40 || val.length() < 8))
				return false;
			else
				return true;
		}
		
		void connect_sta()
		{
			/*if (is_valid_mac(mac_addr))
				wifi_set_macaddr(SOFTAP_IF, &mac_addr[0]);*/
			WiFi.config(settings.sta_ip, settings.sta_gateway, settings.sta_subnet);
			WiFi.begin(settings.sta_ssid, settings.sta_pass);
		}
		
		void connect_ap()
		{
			WiFi.softAPConfig(settings.ap_ip, settings.ap_gateway, settings.ap_subnet);
			WiFi.softAP(settings.ap_ssid, settings.ap_pass);
			/*if (!is_valid_mac(mac_addr))
				WiFi.softAPmacAddress(mac_addr);*/
		}
		
		void begin() //bool sta_enabled
		{
			settings.load();
			//WiFi.persistent(false);
			WiFi.mode(settings.sta_enabled ? WIFI_AP_STA : WIFI_AP);
			//WiFi.mode(WIFI_AP);
			connect_ap();
			if (settings.sta_enabled)
				connect_sta();
		};
		
		void end()
		{
			WiFi.softAPdisconnect();
			WiFi.disconnect();
		}
		
		void begin_scan()
		{
			//last_scan_millis = millis();
			WiFi.scanNetworks(true);
			avail_networks = 0;
			scan = true;
		}
		
		void end_scan()
		{
			scan = false;
		}
		
		String get_station_name(int i)
		{
			/*String probably_incorrect_output = WiFi.SSID(i);
			String correct_output;
			for (int i = 0; i < probably_incorrect_output.length(); i++)
			{
				char c = probably_incorrect_output[i];
				correct_output += isPrintable(c) ? c : '*';
			}
			return correct_output;*/
			return WiFi.SSID(i);
		}
		
		void process_sta()
		{
			static unsigned long old_millis = 0;

			static int sta_connection_attempts = 5; //maximum 5 seconds to connect to sta, then stop.
			static bool sta_connecting = true;
			
			static int sta_reconnect_timeout = 60; //repeat reconnect only after 60 seconds 
			static bool sta_lost_connection = false;

			if (millis() > old_millis + 1000)
				old_millis += 1000;
			else
				return;

			if (WiFi.getMode() == WIFI_AP_STA) {
				if (!sta_lost_connection) {
					if (WiFi.status() != WL_CONNECTED && sta_connecting) {
						if (sta_connection_attempts <= 0) { //if we can't connect then sta is lost connection
							DEBUG_MSG("Connection to sta is lost\n", sta_connection_attempts);
							WiFi.persistent(false);
							WiFi.disconnect(); //quit ESP sta reconnect limbo
							WiFi.persistent(true);
							sta_connection_attempts = 5;
							sta_reconnect_timeout = 60;
							sta_connected = false;
							sta_lost_connection = true;
							sta_connecting = false;
						} else {
							DEBUG_MSG("Connecting to sta (%i)...\n", sta_connection_attempts);
							sta_connection_attempts--;
						}
					}
				} else { //if we lost connection then wait timeout, and then try connect again
					if (sta_reconnect_timeout <= 0) {
						DEBUG_MSG("Sta been disconnected too long, reconnecting...\n");
						sta_connecting = true;
						sta_lost_connection = false; //connection is not lost if we're connecting
						WiFi.begin();
					} else {
						if (!(sta_reconnect_timeout % 10))
							DEBUG_MSG("Timeout to reconnect sta: %i...\n", sta_reconnect_timeout--);
						sta_reconnect_timeout--;
					}
				}
			} else {
				sta_connected = true;
			}
		}

		void loop()
		{	
			if (scan)
				avail_networks = WiFi.scanComplete();
			process_sta();
		}
	}
}
