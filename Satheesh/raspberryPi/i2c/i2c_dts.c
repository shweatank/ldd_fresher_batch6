/dts-v1/;
/plugin/;

/ {
    compatible = "brcm,bcm2711";

    fragment@0 {
        target = <&i2c_arm>;

        __overlay__ {
            #address-cells = <1>;
            #size-cells = <0>;

            rtc@68 {
                compatible = "maxim,ds3231";
                reg = <0x68>;
                status = "okay";
            };
        };
    };
};
