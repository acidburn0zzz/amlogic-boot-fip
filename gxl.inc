O ?= .
TMP ?= .
BL33 ?=
COMPRESS_LZ4 ?= 0

ifeq ($(COMPRESS_LZ4),0)
	BL33_ARGS :=
else
	BL33_ARGS :=--compress lz4
endif

.PHONY: clean distclean
.NOTPARALLEL: ${TMP}/bl30_new.bin ${TMP}/bl2_acs.bin ${TMP}/bl2_new.bin ${TMP}/bl30_new.bin.enc ${TMP}/bl31.img.enc ${TMP}/bl33.bin.enc ${TMP}/bl2.n.bin.sig ${O}/u-boot.bin

all: ${O}/u-boot.bin

clean:
	rm -f ${TMP}/bl30_new.bin ${TMP}/bl2_new.bin
	rm -f ${TMP}/bl30_new.bin.g12a.enc ${TMP}/bl30_new.bin.enc
	rm -f ${TMP}/bl31.img.enc ${TMP}/bl33.bin.enc
	rm -f ${TMP}/bl21_zero.bin ${TMP}/bl2_zero.bin
	rm -f ${TMP}/bl301_zero.bin ${TMP}/bl30_zero.bin ${TMP}/bl2.n.bin.sig

distclean: clean
	rm -f ${O}/u-boot.bin ${O}/u-boot.bin.sd.bin ${O}/u-boot.bin.usb.bl2 ${O}/u-boot.bin.usb.tpl

${TMP}/bl30_new.bin: bl30.bin bl301.bin
	./blx_fix.sh bl30.bin ${TMP}/zero_tmp ${TMP}/bl30_zero.bin bl301.bin ${TMP}/bl301_zero.bin ${TMP}/bl30_new.bin bl30
	@if [ -e ${TMP}/zero_tmp ] ; then rm ${TMP}/zero_tmp ; fi
	@rm ${TMP}/bl30_zero.bin ${TMP}/bl301_zero.bin

${TMP}/bl2_acs.bin: bl2.bin acs.bin
	python3 acs_tool.py bl2.bin ${TMP}/bl2_acs.bin acs.bin 0

${TMP}/bl2_new.bin: ${TMP}/bl2_acs.bin bl21.bin
	./blx_fix.sh ${TMP}/bl2_acs.bin ${TMP}/zero_tmp ${TMP}/bl2_zero.bin bl21.bin ${TMP}/bl21_zero.bin ${TMP}/bl2_new.bin bl2
	@if [ -e ${TMP}/zero_tmp ] ; then rm ${TMP}/zero_tmp ; fi
	@rm ${TMP}/bl2_zero.bin ${TMP}/bl21_zero.bin

${TMP}/bl30_new.bin.enc: ${TMP}/bl30_new.bin
	./aml_encrypt_gxl --bl3enc --input ${TMP}/bl30_new.bin --output ${TMP}/bl30_new.bin.enc

${TMP}/bl31.img.enc: bl31.img
	./aml_encrypt_gxl --bl3enc --input bl31.img --output ${TMP}/bl31.img.enc

${TMP}/bl33.bin.enc: ${BL33}
	./aml_encrypt_gxl --bl3enc --input ${BL33} --output ${TMP}/bl33.bin.enc ${BL33_ARGS}

${TMP}/bl2.n.bin.sig: ${TMP}/bl2_new.bin
	./aml_encrypt_gxl --bl2sig --input ${TMP}/bl2_new.bin --output ${TMP}/bl2.n.bin.sig

${O}/u-boot.bin: ${TMP}/bl2.n.bin.sig ${TMP}/bl30_new.bin.enc ${TMP}/bl31.img.enc ${TMP}/bl33.bin.enc
	./aml_encrypt_gxl --bootmk --output ${O}/u-boot.bin \
	               --bl2 ${TMP}/bl2.n.bin.sig --bl30 ${TMP}/bl30_new.bin.enc \
		       --bl31 ${TMP}/bl31.img.enc --bl33 ${TMP}/bl33.bin.enc
