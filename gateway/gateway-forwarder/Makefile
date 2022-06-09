### Environment constants 

ARCH ?=
CROSS_COMPILE ?=
export

### general build targets

all:
	$(MAKE) all -e -C libloragw
	$(MAKE) all -e -C util_pkt_logger
	$(MAKE) all -e -C util_tx_test
	$(MAKE) all -e -C util_tx_continuous
	$(MAKE) all -e -C lora_pkt_fwd
	$(MAKE) all -e -C gps_hat

clean:
	$(MAKE) clean -e -C libloragw
	$(MAKE) clean -e -C util_pkt_logger
	$(MAKE) clean -e -C util_tx_test
	$(MAKE) clean -e -C util_tx_continuous
	$(MAKE) clean -e -C lora_pkt_fwd
	$(MAKE) clean -e -C gps_hat
	rm -rf lorapktfwd.deb

deb:
	rm -rf pi_pkg
	mkdir -p pi_pkg/usr/bin
	mkdir -p pi_pkg/lib/systemd/system
	mkdir -p pi_pkg/etc/lora-gateway
	cp -rf DEBIAN pi_pkg
	cp -rf lorapktfwd.service pi_pkg/lib/systemd/system
	cp -rf lora_pkt_fwd/cfg pi_pkg/etc/lora-gateway
	cp -rf lora_pkt_fwd/*conf.json pi_pkg/etc/lora-gateway
	cp -rf lora_pkt_fwd/cfg pi_pkg/etc/lora-gateway
	install -m 755 lora_pkt_fwd/lora_pkt_fwd pi_pkg/usr/bin
	install -m 755 util_tx_test/util_tx_test pi_pkg/usr/bin
	install -m 755 util_tx_continuous/util_tx_continuous pi_pkg/usr/bin
	install -m 755 util_pkt_logger/util_pkt_logger pi_pkg/usr/bin
	install -m 755 reset_lgw.sh pi_pkg/usr/bin
	dpkg -b pi_pkg lorapktfwd.deb
	rm -rf pi_pkg


### EOF
