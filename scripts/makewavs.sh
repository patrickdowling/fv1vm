#!/usr/bin/env bash
# This could definitely be more data-driven

function fatal() { echo "$*" ; exit 1; }


BANK_DIR=./banks/wav_tests
OUTPUT_DIR=./build/wav
FV1_WAV=./build/fv1_wav

BANK="$BANK_DIR/build/wav_tests.bank"

[ -x "$FV1_WAV" ] || fatal "$FV1_WAV not executable"

function make_wav() {
	local program=$1
	local p="-p$(echo $1 | cut -d_ -f1)"
	shift
	local s="-s$1"
	shift

	local output_file="$OUTPUT_DIR/$program.wav"

	CMD="$FV1_WAV -f $BANK $p $s --bits_per_sample=24 -o $output_file $*"
	echo $CMD
	$CMD
}

function seconds() {
	echo $((32000 * $1))
}

mkdir -p "$OUTPUT_DIR"
make -C "$BANK_DIR" clean bank
[ -f "$BANK" ] || fatal "$BANK does not exist"

make_wav 0_sincos_ampmax  $(seconds 1)

make_wav 1_rmp_amp4096 $(seconds 1) pot0=1.0 pot1=1.0
make_wav 1_rmp_amp512 $(seconds 1) pot0=1.0 pot1=0.0
make_wav 1_rmp_freq0 $(seconds 1) pot0=0.0 pot1=1.0

make_wav 2_rmp_xfade $(seconds 1)

make_wav 3_xfade_sin $(seconds 2) pot0=0.2 pot2=1.0
