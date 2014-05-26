#!usr/bin/teclsh8.5
package require Tclx
package require Expect

set MAC			[lindex $argv 0];
set VLAN_VID		[lindex $argv 1];
set INTERFACE		[lindex $argv 2];


spawn telnet 10.10.10.61 23
sleep 1

# --------------------- FEC CONFIGURATION --------------------------------------

expect "Username:"
send "i2t\r"
expect "Password:"
send "cisco\r"
expect "Cablemodem#"
send "configure\r"
expect "Configuring from terminal, memory, or network"
send "\r"
expect "Cablemodem(config)"
send "no cable dot1q-vc-map $MAC GigabitEthernet 0/$INTERFACE $VLAN_VID"
expect "Cablemodem(config)"
send "exit\r"
expect "Cablemodem#"
send "exit\r"
expect "Cablemodem#"

puts "\nSuccessfully configured\n" ;# Checkpoint

