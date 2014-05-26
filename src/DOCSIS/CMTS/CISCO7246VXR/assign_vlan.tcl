#!usr/bin/teclsh8.5
package require Tclx
package require Expect

set MAC			[lindex $argv 0];
set VLAN_VID		[lindex $argv 1];
#set INTERFACE		[lindex $argv 2];
set INTERFACE 		1


spawn telnet 158.227.98.6 23
sleep 1

# --------------------- FEC CONFIGURATION --------------------------------------

#expect "Username:"
#send "i2t\r"
expect "Password:"
send "cisco\r"
expect "Cablemodem>"
send "enable\r"
expect "Password:"
send "cisco\r"
expect "Cablemodem#"

send "configure\r"
expect "Configuring from terminal, memory, or network"
send "\r"
expect "Enter configuration commands, one per line.  End with CNTL/Z."
send "\r"
expect "Cablemodem(config)#"
send "\r"
expect "Cablemodem(config)#"
send "cable dot1q-vc-map $MAC GigabitEthernet 0/$INTERFACE $VLAN_VID\r"
expect "Cablemodem(config)"
send "exit\r"
expect "Cablemodem#"
send "exit\r"
expect "Cablemodem#"

puts "\nSuccessfully configured\n" ;# Checkpoint

