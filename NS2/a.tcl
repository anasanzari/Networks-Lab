set ns [new Simulator]
set nf [open out.nam w]
set nt [open tra.tr w]
$ns trace-all $nt
$ns namtrace-all $nf

proc finish {} {
  global ns nf
  $ns flush-trace
  close $nf
  exit 0
}


set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]

$ns duplex-link $n0 $n2 2Mb 10ms DropTail
$ns duplex-link $n1 $n2 2Mb 10ms DropTail
$ns duplex-link $n2 $n3 1.7Mb 20ms DropTail

$ns queue-limit $n0 $n2 10
$ns queue-limit $n1 $n2 10
$ns queue-limit $n2 $n3 10

$ns duplex-link-op $n0 $n2 orient right-down
$ns duplex-link-op $n1 $n2 orient right-up
$ns duplex-link-op $n2 $n3 orient right

set udp [new Agent/UDP]
set null [new Agent/Null]

$ns attach-agent $n1  $udp
$ns attach-agent $n3 $null
$ns connect $udp $null

set tcp [new Agent/TCP]
set tcpsink [new Agent/TCPSink]

$ns attach-agent $n0  $tcp
$ns attach-agent $n3 $tcpsink
$ns connect $tcp $tcpsink


set ftp [new Application/FTP]
$ftp attach-agent $tcp



set cbr [new Application/Traffic/CBR]
$cbr set packetSize_ 1000
$cbr set rate_ 1Mb
$cbr attach-agent $udp




$ns at 0.5 "$cbr start"
$ns at 1.0 "$ftp start"
$ns at 4.0 "$ftp stop"
$ns at 4.5 "$cbr stop"
$ns at 5.0 "finish"

$ns run


