module hc166(
    input  wire       pe_n,     // parallel enable input   
    input  wire       ds,       // serial data input
    input  wire [7:0] d,        // parallel data inputs

    input  wire       mr_n,     // asynchronous master reset
    input  wire       cp,       // clock input
    input  wire       ce_n,     // clock enable input

    output wire       q7);      // serial output

    reg [7:0] data_r;

    always @(posedge cp, negedge mr_n)
        if (!mr_n)
            data_r <= 8'h0;
        else if (!ce_n)
            data_r <= !pe_n ? d : {data_r[6:0], ds};

    assign q7 = data_r[7];

endmodule
