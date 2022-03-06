; Copy ADCL -> DACL, ADCR -> DACR
ldax ADCL
wrax DACL, 0.0
ldax ADCR
wrax DACR, 0.0
