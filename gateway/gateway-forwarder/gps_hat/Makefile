# hat_pkt_fwd

all: hat_pkt_fwd dual_pkt_fwd single_rx_tx

hat_pkt_fwd: base64.o parson.o radio.o jitqueue.o hat_pkt_fwd.o
	$(CC) hat_pkt_fwd.o base64.o parson.o radio.o jitqueue.o -lm -lrt -ldl -lpthread -o hat_pkt_fwd

dual_pkt_fwd: base64.o parson.o radio.o jitqueue.o dual_pkt_fwd.o
	$(CC) dual_pkt_fwd.o base64.o parson.o radio.o jitqueue.o -lm -lrt -ldl -lpthread -o dual_pkt_fwd

single_rx_tx: radio.o
	$(CC) $(CFLAGS) radio.o -lrt -ldl single_rx_tx.c -o single_rx_tx


hat_pkt_fwd.o: hat_pkt_fwd.c
	$(CC) $(CFLAGS) -c hat_pkt_fwd.c

dual_pkt_fwd.o: dual_pkt_fwd.c
	$(CC) $(CFLAGS) -c dual_pkt_fwd.c

jitqueue.o: jitqueue.c
	$(CC) $(CFLAGS) -c jitqueue.c

base64.o: base64.c
	$(CC) $(CFLAGS) -c base64.c

parson.o: parson.c
	$(CC) $(CFLAGS) -c parson.c

radio.o: radio.c
	$(CC) $(CFLAGS) -c radio.c

clean:
	rm -rf *.o hat_pkt_fwd dual_pkt_fwd single_rx_tx 
