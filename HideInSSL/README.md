# HideInSSL

## Description

```
Pictures are hidden in SSL.
(pcap download link)
```

## How to solve

1. If you see the packets, you can find out what many SSL "Client Hello"
packets are sent from 192.168.0.107 to 192.168.0.128 server.
2. Filter the packets with rule "(src host 192.168.0.107 && dst host
192.168.0.128) || (src host 192.168.0.128 && dst host 192.168.0.107)"
3. Picture files are transferred by using SSL covert channel ("Random" field
in Secure Socket Layer). And One file was transferred at each session.
4. And sometimes, when the server responds '0' (not '1'), then the client
needs to resend the same message. On extracting files, data need to be
ignored in that situation.
5. Make file-extractor by considering above points.
