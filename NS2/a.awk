BEGIN {
p = 0;
}
{
 event = $1;
 if(event=="d")
 {
  p++;
 }
}
END {
printf("No of packets : %d", p);
}
