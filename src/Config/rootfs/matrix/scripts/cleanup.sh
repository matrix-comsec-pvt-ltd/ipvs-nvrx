#!/bin/sh

#==================================================================================================
# Script starts from here
#==================================================================================================
# Remove html files
rm -f /matrix/web/html_pages/Logsui.html
rm -f /matrix/web/html_pages/Logview.html
rm -f /matrix/web/html_pages/Logviewui.html
rm -f /matrix/web/html_pages/Maintenance.html
rm -f /matrix/web/html_pages/MxFFMPEG.html
rm -f /matrix/web/html_pages/Satatya.htm
rm -f /matrix/web/html_pages/Login.html
rm -f /matrix/web/html_pages/SessionTimeOut.html
rm -f /matrix/web/html_pages/Settings.html
rm -f /matrix/log/Downloads.html

# Remove js files
rm -f /matrix/web/html_pages/js/telnet.js
rm -f /matrix/web/html_pages/SetLogsui.js
rm -f /matrix/web/html_pages/jquery-ui-i18n.min.js
rm -f /matrix/web/html_pages/Validations.js
rm -f /matrix/web/html_pages/jquery-1.11.0.min.js
rm -f /matrix/web/html_pages/MxTranslator.js
rm -f /matrix/web/html_pages/SetLogs.js

# Remove xml files
rm -f /matrix/web/html_pages/LogConfig.xml
rm -f /matrix/web/html_pages/PcapTrace.xml

# Remove misc files
rm -f /matrix/web/html_pages/SetLogs.css
rm -f /matrix/web/html_pages/MatrixMACServerSettings.css
rm -f /matrix/web/html_pages/SatatyaWenInstaller.png
rm -f /matrix/web/html_pages/SatatyaWebInstaller.png
rm -f /usr/share/fonts/unifont.ttf
rm -f /usr/share/fonts/Japanese.TTF
rm -f /usr/share/fonts/ZCOOLXiaoWei-Regular.ttf
rm -f /matrix/log/*.txt
rm -f /etc/dnsmasq.conf

# Remove unused config files
rm -f /matrix/config/appConfig/Rs232.cfg
rm -f /matrix/config/appConfig/AnalogCamera.cfg
rm -f /matrix/config/appConfig/PtzInterface.cfg
rm -f /matrix/config/appConfig/AudioGain.cfg
rm -f /matrix/config/appConfig/DeviceInfoClient.cfg

# Remove script and binary files
rm -f /matrix/scripts/dhcp.sh
rm -f /matrix/scripts/subRtsp.sh
rm -f /matrix/scripts/addUser.sh
rm -f /matrix/scripts/deleteUser.sh
rm -f /matrix/scripts/storageDevAddRemove.sh
rm -f /matrix/scripts/netServiceStartStop.sh
rm -f /matrix/scripts/telnetServiceStartStop.sh
rm -f /matrix/bin/RTSPClientSub.bin
rm -f /matrix/bin/genericSysCmd.bin
rm -f /matrix/bin/diskSysCmd.bin
rm -f /matrix/bin/netDevDetect.bin
rm -f /matrix/bin/dhcpServerNotify.bin
rm -f /opt/bin/multiNvrClient
rm -f /opt/bin/nvrTestApp

# Remove folders
rm -rf /matrix/web/html_pages/cgi-bin
rm -rf /matrix/web/html_pages/css/
rm -rf /matrix/web/html_pages/html/
rm -rf /matrix/web/html_pages/images/
rm -rf /matrix/web/html_pages/js/
rm -rf /matrix/log/Decoder_state
rm -rf /matrix/config/ModemDatabase
rm -rf /matrix/media/*

# Remove boa web server
rm -f /etc/boa/boa.conf
rm -f /usr/sbin/boa
rm -rf /usr/lib/boa

#==================================================================================================
# End of Script
#==================================================================================================
