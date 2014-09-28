typedef struct{
    unsigned int version:2;
    unsigned int p:1;
    unsigned int x:1;
    unsigned int cc:4;
    unsigned int m:1;
    unsigned int pt:7;

    u_int16 seq;
    u_int32 timestamp;
    u_int32 ssrc;
    u_int32 *csrc;
} rtp_header;

typedef struct
{
    rtp_header header;
    char* payload;
    long payload_len;
} rtp_packet;
