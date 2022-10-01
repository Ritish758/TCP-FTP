set terminal png
set output "Wireless_TCP_Veno.png"
set title "Throughput vs Packet size for TCP-Veno"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP-Veno" with linespoints
40 849.615
44 826.244
48 807.119
52 791.243
60 763.351
552 799.106
576 794.619
628 794.818
1420 1036.4
1500 1036.24
e
