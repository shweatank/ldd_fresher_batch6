/dts-v1/;
/plugin/;

/{
    compatible = "brcm,bcm2711";

    fragment@0 {
        target-path = "/";

        __overlay__ {
            my_led {
                compatible = "prashant,my-led";
                gpios = <&gpio 17 0>;
                status = "okay";
            };
        };
    };
};
