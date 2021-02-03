#ifndef UI_NETWORK_GUARD
#define UI_NETWORK_GUARD

#include "Arduino.h"
#include <LittleFS.h>
#include <ESP8266WiFi.h>

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

//#define SCAN_PERIOD 8000
//extern long unsigned int last_scan_millis;

namespace ui {
	namespace network {
		extern bool sta_connected;
		extern int avail_networks;
		extern bool scan;
		
		struct settings {
			static bool need_commit;
			settings &changed();
			void committed();
		
			bool sta_enabled;
			
			char ap_ssid[80];
			char ap_pass[80];
			uint32_t ap_ip;
			uint32_t ap_gateway;
			uint32_t ap_subnet;
				
			char sta_ssid[80];
			char sta_pass[80];
			uint32_t sta_ip;
			uint32_t sta_gateway;
			uint32_t sta_subnet;
			
			void defaults();
			void save();		
			void load();
			
			settings();
		};
		
		extern settings settings;
		
		extern bool passphrase_is_valid(String);
		extern void connect_sta();
		extern void connect_ap();
		extern void begin();
		extern void end();
		extern void begin_scan();
		extern void end_scan();
		extern String get_station_name(int i);
		extern void process_sta();
		extern void loop();
	}
}

#endif
