//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		TimeZone.c
@brief      This is the file which contains all the time zones Available to the user that can be configured.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "TimeZone.h"
#include "DateTime.h"
#include "SysTimer.h"
#include "DebugLog.h"
#include "HttpClient.h"
#include "Config.h"
#include "EventLogger.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define AUTO_TIMEZONE_DETECT_INTERVAL	300		// (5min * 60) = 300 seconds
#define TRY_AGAIN_INTERVAL              10
#define MAX_TIMEZONE_CONN_TIME          10
#define	TIMEZONE_HTTP_PORT              80
#define MAX_TIMEZONE_LENGTH             30
#define AUTO_TIMEZONE_URL               "ip-api.com"
#define XML_TAG_TIMEZONE                "timezone"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void httpAutoTimezoneDetectCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo);
//-------------------------------------------------------------------------------------------------
static BOOL	startTimeZoneSrch(void);
//-------------------------------------------------------------------------------------------------
static void srchTimeZone(void);
//-------------------------------------------------------------------------------------------------
static void autoTimeZoneTmrCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void tryAgainTmrCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL GetXMLTag(CHARPTR source, CHARPTR tag, CHARPTR dest, UINT16 maxSize);
//-------------------------------------------------------------------------------------------------
static UINT8 getDetectedTmZnIndex(CHARPTR timeZoneName);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
//LOOKUP table of TimeZone Offset
const INT32 TimeZoneinSec[MAX_TIMEZONE] =
{
    -43200,   			//"GMT+12:00",		1. Enivetok,Kwajalein
    -39600,  			//"GMT+11:00",		2. Midway island,samoa
    -36000,  			//"GMT+10:00",		3. Hawaii
    -32400, 			//"GMT+9:00",		4. Alaska
    -28800,  			//"GMT+8:00",		5. Pacific Time(US & Canada),Tijuana
    -25200,   			//"GMT+7:00",		6. Arizona
    -25200,   			//"GMT+7:00",		7. Chihuahua, La Paz, Mazatlan
    -25200,   			//"GMT+7:00",		8. Mountain Time(US & Canada)
    -21600,  			//"GMT+6:00",		9. Central America
    -21600,  			//"GMT+6:00",		10. Central time(US & Canada)
    -21600,  			//"GMT+6:00",		11. Guadalajara, mexico City, Monterrey
    -21600,  			//"GMT+6:00",		12. Saskatchewan
    -18000, 			//"GMT+5:00",		13. Bogota, Lima, Quito
    -18000, 			//"GMT+5:00",		14. Eastern Time(US & Canada)
    -18000, 			//"GMT+5:00",		15. Indiana (East)
    -14400,  			//"GMT+4:00",		16. Atlantic Time(Canada)
    -14400,  			//"GMT+4:00",		17. Caracas, La Paz
    -14400,  			//"GMT+4:00",		18. Santiago
    -12600,  			//"GMT+3:30",		19. Newfoundland
    -10800,  			//"GMT+3:00",		20. Brasilia
    -10800,  			//"GMT+3:00",		21. Buenos Aires, Georgetown
    -10800,  			//"GMT+3:00",		22. Greenland
    -7200,				//"GMT+2:00",		23. Mid atlantic
    -3600,  			//"GMT+1:00",		24. Azores
    -3600,  			//"GMT+1:00",		25. Cape verde ls
    0,   				//"GMT",  			26. Casablanca, Monrovia
    0,   				//"GMT",  			27. Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London
    +3600,				//"GMT-1:00",		28. Amsterdam, Berlin, Bern, Rome, stockholm, Vienna
    +3600,				//"GMT-1:00",		29. Belgrade, Bratislava, Budapest, Ljubljana, Prague
    +3600,				//"GMT-1:00",		30. Brussels,Copenhagen,Madrid,Paris
    +3600,				//"GMT-1:00",		31. West Central Africa
    +3600,				//"GMT-1:00",		32. Sarajevo, Skopje, Warsaw, Zagreb
    +7200,				//"GMT-2:00",		33. Athens,Istanbul,Minsk
    +7200,				//"GMT-2:00",		34. Bucharest
    +7200,				//"GMT-2:00",		35. Cairo
    +7200,				//"GMT-2:00",		36. Harare, Pretoria
    +7200,				//"GMT-2:00",		37. Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius
    +7200,				//"GMT-2:00",		38. Jerusalem
    +10800,				//"GMT-3:00",		39. Baghdad
    +10800,				//"GMT-3:00",		40. Kuwait, Riyadh
    +10800,				//"GMT-3:00",		41.	Moscow, St. Petersburg, Volgograd
    +10800,				//"GMT-3:00",		42. Nairobi
    +12600,				//"GMT-3:30",		43. Tehran
    +14400,				//"GMT-4:00",		44. Abu Dhabi,Maskat
    +14400,				//"GMT-4:00",		45. Baku, Tbilisi, Yerevan
    +16200,				//"GMT-4:30",		46. Kabul
    +18000,				//"GMT-5:00",		47. Ekaterinburg
    +18000,				//"GMT-5:00",		48. Islamabad,Karanchi,Tashkent
    +19800,				//"GMT-5:30",		49. Culcutta,Chennai,Mumbai,New Delhi
    +20700,				//"GMT-5:45",		50. Kathmandu
    +21600,				//"GMT-6:00",		51. Almaty, Novosibirsk
    +21600,				//"GMT-6:00",		52. Astana,Dhaka
    +21600,				//"GMT-6:00",		53. Sri Jayawardenepura
    +23400,				//"GMT-6:30",		54. Rangoon
    +25200,				//"GMT-7:00",		55. Bangtok,Hanoi,Jakarta
    +25200,				//"GMT-7:00",		56.	Krasnoyarsk
    +28800,				//"GMT-8:00",		57. Beijing, Chongqing, Hong Kong, Urumqi
    +28800,				//"GMT-8:00",		58.	Irkutsk, Ulaan Bataar
    +28800,				//"GMT-8:00",		59. Kaula Lumpur,Singapore
    +28800,				//"GMT-8:00",		60.	Perth
    +28800,				//"GMT-8:00",		61.	Taipei
    +32400,				//"GMT-9:00",		62. Osaka,Sapporo,Tokyo
    +32400,				//"GMT-9:00",		63.	Seoul
    +32400,				//"GMT-9:00",		64.	Yakutsk
    +34200,				//"GMT-9:30",		65.	Adelaide
    +34200,				//"GMT-9:30",		66.	 Darwin
    +36000,				//"GMT-10:00",		67. Brisbane
    +36000,				//"GMT-10:00",		68. Canberra, Melbourne, Sydney
    +36000,				//"GMT-10:00",		69. Guam, Port Moresby
    +36000,				//"GMT-10:00",		70. Hobart
    +36000,				//"GMT-10:00",		71. Vladivostok
    +39600,				//"GMT-11:00",		72. Magadan,Solomon Ls,New Calendonia
    +43200,				//"GMT-12:00",		73. Auckland,Wellington
    +43200,				//"GMT-12:00",		74. Fiji, Kamchatka, Marshall Is.
    +46800				//"GMT-13:00"		75.  Nuku'alofa
};

const UINT32 RegionalInfo[MAX_TIMEZONE][MAX_REG_INFO]=
{
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT+12:00",		1. Enivetok,Kwajalein
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+11:00",		2. Midway island,samoa
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+10:00",		3. Hawaii
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT+9:00",		4. Alaska
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+8:00",		5. Pacific Time(US & Canada),Tijuana
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+7:00",		6. Arizona
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT+7:00",		7. Chihuahua, La Paz, Mazatlan
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+7:00",		8. Mountain Time(US & Canada)
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+6:00",		9. Central America
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+6:00",		10. Central time(US & Canada)
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT+6:00",		11. Guadalajara, mexico City, Monterrey
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+6:00",		12. Saskatchewan
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT+5:00",		13. Bogota, Lima, Quito
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+5:00",		14. Eastern Time(US & Canada)
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+5:00",		15. Indiana (East)
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+4:00",		16. Atlantic Time(Canada)
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT+4:00",		17. Caracas, La Paz
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT+4:00",		18. Santiago
    {VIDEO_SYS_NTSC,DATE_FORMAT_MMDDYYY},	//"GMT+3:30",		19. Newfoundland
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT+3:00",		20. Brasilia
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT+3:00",		21. Buenos Aires, Georgetown
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT+3:00",		22. Greenland
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT+2:00",		23. Mid atlantic
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT+1:00",		24. Azores
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT+1:00",		25. Cape verde ls
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT",  			26. Casablanca, Monrovia
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT",  			27. Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-1:00",		28. Amsterdam, Berlin, Bern, Rome, stockholm, Vienna
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-1:00",		29. Belgrade, Bratislava, Budapest, Ljubljana, Prague
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-1:00",		30. Brussels,Copenhagen,Madrid,Paris
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-1:00",		31. West Central Africa
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-1:00",		32. Sarajevo, Skopje, Warsaw, Zagreb
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-2:00",		33. Athens,Istanbul,Minsk
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-2:00",		34. Bucharest
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-2:00",		35. Cairo
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-2:00",		36. Harare, Pretoria
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-2:00",		37. Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-2:00",		38. Jerusalem
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-3:00",		39. Baghdad
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-3:00",		40. Kuwait, Riyadh
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-3:00",		41.	Moscow, St. Petersburg, Volgograd
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-3:00",		42. Nairobi
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-3:30",		43. Tehran
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-4:00",		44. Abu Dhabi,Maskat
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-4:00",		45. Baku, Tbilisi, Yerevan
    {VIDEO_SYS_PAL,DATE_FORMAT_YYYYMMDD},	//"GMT-4:30",		46. Kabul
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-5:00",		47. Ekaterinburg
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-5:00",		48. Islamabad,Karanchi,Tashkent
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-5:30",		49. Culcutta,Chennai,Mumbai,New Delhi
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-5:45",		50. Kathmandu
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-6:00",		51. Almaty, Novosibirsk
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-6:00",		52. Astana,Dhaka
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-6:00",		53. Sri Jayawardenepura
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT-6:30",		54. Rangoon
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT-7:00",		55. Bangtok,Hanoi,Jakarta
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-7:00",		56.	Krasnoyarsk
    {VIDEO_SYS_PAL,DATE_FORMAT_YYYYMMDD},	//"GMT-8:00",		57. Beijing, Chongqing, Hong Kong, Urumqi
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-8:00",		58.	Irkutsk, Ulaan Bataar
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-8:00",		59. Kaula Lumpur,Singapore
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-8:00",		60.	Perth
    {VIDEO_SYS_NTSC,DATE_FORMAT_YYYYMMDD},	//"GMT-8:00",		61.	Taipei
    {VIDEO_SYS_NTSC,DATE_FORMAT_YYYYMMDD},	//"GMT-9:00",		62. Osaka,Sapporo,Tokyo
    {VIDEO_SYS_NTSC,DATE_FORMAT_YYYYMMDD},	//"GMT-9:00",		63.	Seoul
    {VIDEO_SYS_PAL,DATE_FORMAT_YYYYMMDD},	//"GMT-9:00",		64.	Yakutsk
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-9:30",		65.	Adelaide
    {VIDEO_SYS_PAL,DATE_FORMAT_MMDDYYY},	//"GMT-9:30",		66.	 Darwin
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-10:00",		67. Brisbane
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-10:00",		68. Canberra, Melbourne, Sydney
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT-10:00",		69. Guam, Port Moresby
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-10:00",		70. Hobart
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-10:00",		71. Vladivostok
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-11:00",		72. Magadan,Solomon Ls,New Calendonia
    {VIDEO_SYS_PAL,DATE_FORMAT_DDMMYYY},	//"GMT-12:00",		73. Auckland,Wellington
    {VIDEO_SYS_NTSC,DATE_FORMAT_DDMMYYY},	//"GMT-12:00",		74. Fiji, Kamchatka, Marshall Is.
    {VIDEO_SYS_PAL,DATE_FORMAT_YYYYMMDD} 	//"GMT-13:00"		75.  Nuku'alofa
};

const CHARPTR OnviftimeZoneNames[MAX_TIMEZONE][2] =
{
    {"DatelineStandardTime12"       ,"CST+12:00"},
    {"SamoaStandardTime11"          ,"CST+11:00"},
    {"HawaiianStandardTime10"       ,"CST+10:00"},
    {"AlaskanStandardTime9"         ,"CST+9:00"},
    {"PacificStandardTime8"         ,"CST+8:00"},
    {"USMountainStandardTime7"      ,"CST+7:00"},
    {"<MexicoStandardTime>7"        ,"CST+7:00"},
    {"MountainStandardTime7 "       ,"CST+7:00"},
    {"CentralAmericaStandardTime6"  ,"CST+6:00"},
    {"CentralStandardTime6"         ,"CST+6:00"},
    {"MexicoStandardTime6"          ,"CST+6:00"},
    {"CanadaCentralStandardTime6"   ,"CST+6:00"},
    {"SAPacificStandardTime5"       ,"CST+5:00"},
    {"EasternStandardTime5"         ,"CST+5:00"},
    {"USEasternStandardTime5"       ,"CST+5:00"},
    {"AtlanticStandardTime4"        ,"CST+4:00"},
    {"SAWesternStandardTime4"       ,"CST+4:00"},
    {"PacificSAStandardTime4"       ,"CST+4:00"},
    {"NewfoundlandStandardTime3:30" ,"CST+3:30"},
    {"ESouthAmericaStandardTime3"   ,"CST+3:00"},
    {"SAEasternStandardTime3"       ,"CST+3:00"},
    {"GreenlandStandardTime3"       ,"CST+3:00"},
    {"<Mid-AtlanticStandardTime>2"  ,"CST+2:00"},
    {"AzoresStandardTime1"          ,"CST+1:00"},
    {"CapeVerdeStandardTime1"       ,"CST+1:00"},
    {"GreenwichStandardTime0"       ,"CST+0:00"},
    {"GMTStandardTime0"             ,"CST-0:00"},
    {"WEuropeStandardTime-1"        ,"CST-1:00"},
    {"CentralEuropeStandardTime-1"  ,"CST-1:00"},
    {"RomanceStandardTime-1"        ,"CST-1:00"},
    {"CentralEuropeanStandardTime-1","CST-1:00"},
    {"WCentralAfricaStandardTime-1" ,"CST-1:00"},
    {"GTBStandardTime-2"            ,"CST-2:00"},
    {"EEuropeStandardTime-2"        ,"CST-2:00"},
    {"EgyptStandardTime-2"          ,"CST-2:00"},
    {"SouthAfricaStandardTime-2"    ,"CST-2:00"},
    {"FLEStandardTime-2"            ,"CST-2:00"},
    {"IsraelStandardTime-2"         ,"CST-2:00"},
    {"ArabicStandardTime-3"         ,"CST-3:00"},
    {"ArabStandardTime-3"           ,"CST-3:00"},
    {"RussianStandardTime-3"        ,"CST-3:00"},
    {"EAfricaStandardTime-3"        ,"CST-3:00"},
    {"IranStandardTime-3:30"        ,"CST-3:30"},
    {"ArabianStandardTime-4"        ,"CST-4:00"},
    {"CaucasusStandardTime-4"       ,"CST-4:00"},
    {"AfghanistanStandardTime-4:30" ,"CST-4:30"},
    {"EkaterinburgStandardTime-5"   ,"CST-5:00"},
    {"WestAsiaStandardTime-5"       ,"CST-5:00"},
    {"IndiaStandardTime-5:30"       ,"CST-5:30"},
    {"NepalStandardTime-5:45"       ,"CST-5:45"},
    {"NCentralAsiaStandardTime-6"   ,"CST-6:00"},
    {"CentralAsiaStandardTime-6"    ,"CST-6:00"},
    {"SriLankaStandardTime-6"       ,"CST-6:00"},
    {"MyanmarStandardTime-6:30"     ,"CST-6:30"},
    {"SEAsiaStandardTime-7"         ,"CST-7:00"},
    {"NorthAsiaStandardTime-7"      ,"CST-7:00"},
    {"ChinaStandardTime-8"          ,"CST-8:00"},
    {"NorthAsiaEastStandardTime-8"  ,"CST-8:00"},
    {"SingaporeStandardTime-8"      ,"CST-8:00"},
    {"WAustraliaStandardTime-8"     ,"CST-8:00"},
    {"TaipeiStandardTime-8"         ,"CST-8:00"},
    {"TokyoStandardTime-9"          ,"CST-9:00"},
    {"KoreaStandardTime-9"          ,"CST-9:00"},
    {"YakutskStandardTime-9"        ,"CST-9:00"},
    {"CenAustraliaStandardTime-9:30","CST-9:30"},
    {"AUSCentralStandardTime-9:30"  ,"CST-9:30"},
    {"EAustraliaStandardTime-10"    ,"CST-10:00"},
    {"AUSEasternStandardTime-10"    ,"CST-10:00"},
    {"WestPacificStandardTime-10"   ,"CST-10:00"},
    {"TasmaniaStandardTime-10"      ,"CST-10:00"},
    {"VladivostokStandardTime-10"   ,"CST-10:00"},
    {"CentralPacificStandardTime-11","CST-11:00"},
    {"NewZealandStandardTime-12"    ,"CST-12:00"},
    {"FijiStandardTime-12"          ,"CST-12:00"},
    {"TongaStandardTime-13"         ,"CST-13:00"}
};

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static BOOL 			isSrchProcessRunning = FALSE;
static TIMER_HANDLE		autoTimeZoneTmrHandle;
static TIMER_HANDLE		tryAgainTmrHandle;
static const CHARPTR    timeZoneNames[MAX_AUTO_TIMEZONE_NAMES][MAX_AUTO_TIMEZONE_INFO] =
{
    {"Enivetok","1"},						//1		//"GMT+12:00",		1. Enivetok,Kwajalein
    {"Kwajalein","1"},						//2
    {"International Date Line West","1"},	//3
    {"Midway island","2"},					//4		//"GMT+11:00",		2. Midway island,samoa
    {"samoa","2"},							//5
    {"Hawaii","3"},							//6		//"GMT+10:00",		3. Hawaii
    {"Alaska","4"}, 						//7		//"GMT+9:00",		4. Alaska
    {"Pacific Time","5"}, 					//8		//"GMT+8:00",		5. Pacific Time(US & Canada),Tijuana
    {"Tijuana","5"},						//9
    {"Arizona","6"},   						//10	//"GMT+7:00",		6. Arizona
    {"Chihuahua","7"},   					//11	//"GMT+7:00",		7. Chihuahua, La Paz, Mazatlan
    {"La Paz","7"},							//12
    {"Mazatlan","7"},						//13
    {"Mountain Time","8"},   				//14	//"GMT+7:00",		8. Mountain Time(US & Canada)
    {"Central America","9"},  				//15	//"GMT+6:00",		9. Central America
    {"Central time","10"},  				//16	//"GMT+6:00",		10. Central time(US & Canada)
    {"Guadalajara","11"},				  	//17	//"GMT+6:00",		11. Guadalajara, mexico City, Monterrey
    {"mexico City","11"},					//18
    {"Monterrey","11"},						//19
    {"Saskatchewan","12"},			  		//20	//"GMT+6:00",		12. Saskatchewan
    {"Bogota","13"}, 						//21	//"GMT+5:00",		13. Bogota, Lima, Quito
    {"Lima","13"},							//22
    {"Quito","13"},							//23
    {"Eastern Time","14"}, 					//24	//"GMT+5:00",		14. Eastern Time(US & Canada)
    {"Indiana","15"}, 						//25	//"GMT+5:00",		15. Indiana (East)
    {"Atlantic Time","16"},  				//26	//"GMT+4:00",		16. Atlantic Time(Canada)
    {"Caracas","17"},  						//27	//"GMT+4:00",		17. Caracas, La Paz
    {"La Paz","17"},						//28
    {"Santiago","18"},  					//29	//"GMT+4:00",		18. Santiago
    {"Newfoundland","19"},					//30	//"GMT+3:30",		19. Newfoundland
    {"Brasilia","20"},						//31	//"GMT+3:00",		20. Brasilia
    {"Buenos-Aires","21"},  				//32	//"GMT+3:00",		21. Buenos-Aires, Georgetown
    {"Georgetown","21"},					//33
    {"Greenland","22"},  					//34	//"GMT+3:00",		22. Greenland
    {"Mid-atlantic","23"},					//35	//"GMT+2:00",		23. Mid-atlantic
    {"Azores","24"},  						//36	//"GMT+1:00",		24. Azores
    {"Cape verde ls","25"},  				//37	//"GMT+1:00",		25. Cape verde ls
    {"Casablanca","26"}, 					//38	//"GMT",  			26. Casablanca, Monrovia
    {"Monrovia","26"},						//39
    {"Dublin","27"},						//40   	//"GMT",  			27. Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London
    {"Edinburgh","27"},						//41
    {"Lisbon","27"},						//42
    {"London","27"},						//43
    {"Amsterdam","28"},						//44	//"GMT-1:00",		28. Amsterdam, Berlin, Bern, Rome, stockholm, Vienna
    {"Berlin","28"},						//45
    {"Bern","28"},							//46
    {"Rome","28"},							//47
    {"stockholm","28"},						//48
    {"Vienna","28"},						//49
    {"Belgrade","29"},						//50	//"GMT-1:00",		29. Belgrade, Bratislava, Budapest, Ljubljana, Prague
    {"Bratislava","29"},					//51
    {"Budapest","29"},						//52
    {"Ljubljana","29"},						//53
    {"Prague","29"},						//54
    {"Brussels","30"},						//55	//"GMT-1:00",		30. Brussels,Copenhagen,Madrid,Paris
    {"Copenhagen","30"},					//56
    {"Madrid","30"},						//57
    {"Paris","30"},							//58
    {"West Central Africa","31"},			//59	//"GMT-1:00",		31. West Central Africa
    {"Sarajevo","32"},						//60	//"GMT-1:00",		32. Sarajevo, Skopje, Warsaw, Zagreb
    {"Skopje","32"},						//61
    {"Warsaw","32"},						//62
    {"Zagreb","32"},						//63
    {"Athens","33"},						//64	//"GMT-2:00",		33. Athens,Beirut,Istanbul,Minsk
    {"Beirut","33"},						//65
    {"Istanbul","33"},						//66
    {"Minsk","33"},							//67
    {"Bucharest","34"},						//68	//"GMT-2:00",		34. Bucharest
    {"Cairo","35"},							//69	//"GMT-2:00",		35. Cairo
    {"Harare","36"},						//70	//"GMT-2:00",		36. Harare, Pretoria
    {"Pretoria","36"},						//71
    {"Helsinki","37"},						//72	//"GMT-2:00",	37. Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius
    {"Kyiv","37"},							//73
    {"Riga","37"},							//74
    {"Sofia","37"},							//75
    {"Tallinn","37"},						//76
    {"Vilnius","37"},						//77
    {"Jerusalem","38"},						//78	//"GMT-2:00",		38. Jerusalem
    {"Baghdad","39"},						//79	//"GMT-3:00",		39. Baghdad
    {"Kuwait","40"},						//80	//"GMT-3:00",		40. Kuwait, Riyadh
    {"Riyadh","40"},						//81
    {"Moscow","41"},						//82	//"GMT-3:00",		41.	Moscow, St. Petersburg, Volgograd
    {"St Petersburg","41"},					//83
    {"Nairobi","42"},						//84	//"GMT-3:00",		42. Nairobi
    {"Tehran","43"},						//85	//"GMT-3:30",		43. Tehran
    {"Abu Dhabi","44"},						//86	//"GMT-4:00",		44. Abu Dhabi,Muscat
    {"Muscat","44"},						//87
    {"Baku","45"},							//88	//"GMT-4:00",		45. Baku, Tbilisi, Yerevan
    {"Tbilisi","45"},						//89
    {"Yerevan","45"},						//90
    {"Kabul","46"},							//91	//"GMT-4:30",		46. Kabul
    {"Ekaterinburg","47"},					//92	//"GMT-5:00",		47. Ekaterinburg
    {"Islamabad","48"},						//93	//"GMT-5:00",		48. Islamabad,Karachi,Tashkent
    {"kolkata","49"},						//94	//"GMT-5:30",		49. kolkata,Chennai,Mumbai,New Delhi
    {"Chennai","49"},						//95
    {"Mumbai","49"},						//96
    {"New Delhi","49"},						//97
    {"Kathmandu","50"},						//98	//"GMT-5:45",		50. Kathmandu
    {"Almay","51"},							//99	//"GMT-6:00",		51. Almay, Novosibirsk
    {"Novosibirsk","51"},					//100
    {"Astana","52"},						//101	//"GMT-6:00",		52. Astana,Dhaka
    {"Dhaka","52"},							//102
    {"Sri Jayawardenepura","53"},			//103	//"GMT-6:00",		53. Sri Jayawardenepura
    {"Rangoon","54"},						//104	//"GMT-6:30",		54. Rangoon
    {"Bangtok","55"},						//105	//"GMT-7:00",		55. Bangtok,Hanoi,Jakarta
    {"Hanoi","55"},							//106
    {"Jakarta","55"},						//107
    {"Krasnoyarsk","56"},					//108	//"GMT-7:00",		56.	Krasnoyarsk
    {"Beijing","57"},						//109	//"GMT-8:00",		57. Beijing, Chongqing, Hong Kong, Urumqi
    {"Chongqing","57"},						//110
    {"Hong Kong","57"},						//111
    {"Urumqi","57"},						//112
    {"Irkutsk","58"},						//113	//"GMT-8:00",		58.	Irkutsk, Ulaanbataar
    {"Ulaanbataar","58"},					//114
    {"Kuala Lumpur","59"},					//115	//"GMT-8:00",		59. Kuala Lumpur,Singapore
    {"Singapore","59"},						//116
    {"Perth","60"},							//117	//"GMT-8:00",		60.	Perth
    {"Taipei","61"},						//118	//"GMT-8:00",		61.	Taipei
    {"Osaka","62"},							//119	//"GMT-9:00",		62. Osaka,Sapporo,Tokyo
    {"Sapporo","62"},						//120
    {"Tokyo","62"},							//121
    {"Seoul","63"},							//122	//"GMT-9:00",		63.	Seoul
    {"Yakutsk","64"},						//123	//"GMT-9:00",		64.	Yakutsk
    {"Adelaide","65"},						//124	//"GMT-9:30",		65.	Adelaide
    {"Darwin","66"},						//125	//"GMT-9:30",		66.	 Darwin
    {"Brisbane","67"},						//126	//"GMT-10:00",		67. Brisbane
    {"Canberra","68"},						//127	//"GMT-10:00",		68. Canberra, Melbourne, Sydney
    {"Melbourne","68"},						//128
    {"Sydney","68"},						//129
    {"Guam","69"},							//130	//"GMT-10:00",		69. Guam, Port Moresby
    {"Port Moresby","69"},					//131
    {"Hobart","70"},						//132	//"GMT-10:00",		70. Hobart
    {"Vladivostok","71"},					//133	//"GMT-10:00",		71. Vladivostok
    {"Magadan","72"},						//134	//"GMT-11:00",		72. Magadan,Solomon Ls,New Caledonia
    {"Solomon Ls","72"},					//135
    {"New Caledonia","72"},					//136
    {"Auckland","73"},						//137	//"GMT-12:00",		73. Auckland,Wellington
    {"Wellington","73"},					//138
    {"Fiji","74"},							//139	//"GMT-12:00",		74. Fiji, Kamchatka, Marshall Is.
    {"Kamchatka","74"},						//140
    {"Marshall Is","74"},					//141
    {"Nuku'alofa","75"}						//142	//"GMT-13:00"		75.  Nuku'alofa
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes auto timezone search on dongle is gets connected
 */
void InitAutoTimezoneSrch(void)
{
    if(isSrchProcessRunning == TRUE)
    {
        DPRINT(DATE_TIME, "timezone search process already running");
        return;
    }

    TIMER_INFO_t timerInfo;
    isSrchProcessRunning = TRUE;
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(AUTO_TIMEZONE_DETECT_INTERVAL);
    timerInfo.data = 0;
    timerInfo.funcPtr = autoTimeZoneTmrCb;
    StartTimer(timerInfo, &autoTimeZoneTmrHandle);
    srchTimeZone();
    DPRINT(DATE_TIME, "timezone search process started");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops auto timezone search on dongle disconnection
 */
void DeinitAutoTimezoneSrch(void)
{
	if(autoTimeZoneTmrHandle != INVALID_TIMER_HANDLE)
	{
		DeleteTimer(&autoTimeZoneTmrHandle);
	}

	if(tryAgainTmrHandle != INVALID_TIMER_HANDLE)
	{
		DeleteTimer(&tryAgainTmrHandle);
	}

	isSrchProcessRunning = FALSE;
    DPRINT(DATE_TIME, "timezone search process stopped");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get timezone string (Format: +0530)
 * @param   timezoneStr
 */
void GetTimezoneStr(CHAR *timezoneStr)
{
    UINT8   timzoneIdx = GetSystemTimezone();
    CHAR    sign = ' ';
    UINT32  hH = 0, mM = 0;

    /* Set default timezone if invalid found */
    if ((timzoneIdx == 0) || (timzoneIdx > MAX_TIMEZONE))
    {
        timzoneIdx = DFLT_TIMEZONE;
    }

    sscanf(OnviftimeZoneNames[timzoneIdx-1][1], "CST%c%d:%d", &sign, &hH, &mM);
    snprintf(timezoneStr, 6, "%c%02u%02u", (sign == '-') ? '+' : '-', (UINT8)hH, (UINT8)mM);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Search timezone on timer expiry
 */
static void srchTimeZone(void)
{
    if (isSrchProcessRunning == FALSE)
	{
        DPRINT(DATE_TIME, "timezone search process not running");
        return;
    }

    TIMER_INFO_t timerInfo;
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(TRY_AGAIN_INTERVAL);
    timerInfo.data = 0;
    timerInfo.funcPtr = tryAgainTmrCb;
    StartTimer(timerInfo, &tryAgainTmrHandle);
    startTimeZoneSrch();
    DPRINT(DATE_TIME, "timezone search process retry started");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate timezone search request
 * @return  SUCCESS / FAIL
 */
static BOOL startTimeZoneSrch(void)
{
	HTTP_INFO_t		httpInfo;
	HTTP_HANDLE		httpHandle;
	UINT32			httpUserData = 0;

    memset(&httpInfo, 0 , sizeof(httpInfo));
	httpInfo.authMethod = AUTH_TYPE_BASIC;
	httpInfo.maxConnTime = MAX_TIMEZONE_CONN_TIME;
	httpInfo.maxFrameTime = MAX_TIMEZONE_CONN_TIME;
	httpInfo.httpUsrPwd.password[0] = '\0';
	httpInfo.httpUsrPwd.username[0] = '\0';
	httpInfo.userAgent = CURL_USER_AGENT;
    httpInfo.interface = INTERFACE_USB_MODEM;
    snprintf(httpInfo.ipAddress, MAX_CAMERA_ADDRESS_WIDTH, AUTO_TIMEZONE_URL);
    snprintf(httpInfo.relativeUrl, MAX_RELATIVE_URL_WIDTH, "/xml/");
	httpInfo.port = TIMEZONE_HTTP_PORT;

    if(FAIL == StartHttp(GET_REQUEST, &httpInfo, httpAutoTimezoneDetectCb, httpUserData, &httpHandle))
	{
        EPRINT(DATE_TIME, "fail to start timezone search");
        return FAIL;
	}

    DPRINT(DATE_TIME, "send timezone search request");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is timer callback to give signal
 * @param   data
 */
static void autoTimeZoneTmrCb(UINT32 data)
{
    DPRINT(DATE_TIME, "auto timezone search ended");
	DeinitAutoTimezoneSrch();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is timer callback to give signal
 * @param   data
 */
static void tryAgainTmrCb(UINT32 data)
{
    DPRINT(DATE_TIME, "auto timezone search retry timer cb");
	DeleteTimer(&tryAgainTmrHandle);
	srchTimeZone();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   httpAutoTimezoneDetectCb
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpAutoTimezoneDetectCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
    CHARPTR             startPtr;
    CHAR                timeZone[MAX_TIMEZONE_LENGTH];
    CHAR                newTmZnIndex[4];
    CHAR                advDetail[3];
    UINT8               detectedTmZnIndex;
    DATE_TIME_CONFIG_t  userCopy;

    DPRINT(DATE_TIME,"http Response[%d]",(UINT8)dataInfo->httpResponse);
	switch (dataInfo->httpResponse)
	{
        case HTTP_SUCCESS:
        {
            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                break;
            }

            if(tryAgainTmrHandle != INVALID_TIMER_HANDLE)
            {
                DeleteTimer(&tryAgainTmrHandle);
            }

            startPtr = dataInfo->storagePtr;
            if (FAIL == GetXMLTag(startPtr, XML_TAG_TIMEZONE, timeZone, MAX_TIMEZONE_LENGTH))
            {
                EPRINT(DATE_TIME, "fail to get timezone xml tag");
                break;
            }

            detectedTmZnIndex = getDetectedTmZnIndex(timeZone);
            ReadDateTimeConfig(&userCopy);
            DPRINT(DATE_TIME, "timezone info: [tag=%s], [detectedTimezoneIdx=%d], [currentTimezoneIdx=%d]",
                   timeZone, detectedTmZnIndex, userCopy.timezone);
            if(detectedTmZnIndex != userCopy.timezone)
            {
                snprintf(newTmZnIndex, sizeof(newTmZnIndex), "%3d", detectedTmZnIndex);
                snprintf(advDetail, sizeof(advDetail), "%1d%1d", RegionalInfo[detectedTmZnIndex-1][0], RegionalInfo[detectedTmZnIndex-1][1]);
                WriteEvent(LOG_SYSTEM_EVENT, LOG_TIME_ZONE_UPDATE, newTmZnIndex, advDetail, EVENT_ALERT);
            }

            DeinitAutoTimezoneSrch();
        }
        break;

        case HTTP_ERROR:
        case HTTP_CLOSE_ON_ERROR:
        case HTTP_CLOSE_ON_SUCCESS:
        default:
        {
            /* Nothing to do */
        }
        break;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function gives value of requested tag from given source XML data.
 * @param   source
 * @param   tag
 * @param   dest
 * @param   maxSize
 * @return  SUCCESS / FAIL
 */
BOOL GetXMLTag(CHARPTR source, CHARPTR tag, CHARPTR dest, UINT16 maxSize)
{
	CHARPTR	startPtr;
	CHARPTR	endPtr;

	// Find Tag in Source
    startPtr = strcasestr(source, tag);
    if (startPtr == NULL)
	{
        return FAIL;
    }

    startPtr += strlen(tag);
    // Check if It Valid Tag Value Start
    if (startPtr[0] != '>')
    {
        return FAIL;
    }

    // Find End of Tag
    startPtr++;
    endPtr = strstr(startPtr, "</");
    if (endPtr == NULL)
    {
        return FAIL;
    }

    // Copy Tag
    if ((endPtr - startPtr) < maxSize)
    {
        maxSize = (endPtr - startPtr);
    }
    else
    {
        maxSize--;
    }

    strncpy(dest, startPtr, maxSize);
    dest[maxSize] = '\0';
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get timezone index from timezone string
 * @param   timeZoneName
 * @return
 */
UINT8 getDetectedTmZnIndex(CHARPTR timeZoneName)
{
    UINT8 loop;

    for(loop = 0; loop < MAX_AUTO_TIMEZONE_NAMES; loop++)
	{
		if(strcasestr(timeZoneName,timeZoneNames[loop][0]) != NULL)
		{
            return atoi(timeZoneNames[loop][1]);
		}
	}

    return 0;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
