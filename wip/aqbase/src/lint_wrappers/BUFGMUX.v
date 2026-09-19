`timescale  1 ps / 1 ps

module BUFGMUX (O, I0, I1, S);

    parameter CLK_SEL_TYPE = "SYNC";
    output O;
    input  I0, I1, S;

    reg q0, q1;
    reg q0_enable, q1_enable;
    wire q0_t, q1_t;

endmodule
