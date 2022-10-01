set terminal png
set output "Wireless_TCP_Vegas.png"
set title "Throughput vs Packet size for TCP-Vegas"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP-Vegas" with linespoints
40 589.619
44 697.99
48 593.52
52 791.243
60 763.351
552 799.106
576 794.619
628 794.818
1420 1036.4
1500 1036.24
e
