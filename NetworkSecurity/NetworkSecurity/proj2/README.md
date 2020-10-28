# Project2 - Exploit SSL vulnerability 

### Get certificate raw bytes from pcapng files
### Conver der file to cert pem file using openssl
```
  $ openssl x509 -inform der -in in_file.der -outform pem -out out_file.pem
```
### Conver cert pem to pubic key pem
```
  $ openssl x506 -pubkey -noout -in in.pem > pubkey.pem
```

### Do common factorizing between these public files
```
  $ ./common_factorize.py
```

### Get private key
```
  $ ./retrieve_key.py
```

