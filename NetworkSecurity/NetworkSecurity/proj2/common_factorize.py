#!/usr/bin/python

from Crypto.PublicKey import RSA
from fractions import gcd
pems = []
for i in range(1, 11):
    p = open("pems/pubkey_" + str(i) + ".pem").read()
    pems.append(RSA.importKey(p))

for i in range(0, 10):
    for j in range (i+1, 10):
        n = gcd(pems[i].n, pems[j].n)
        if  n != 1:
            print (str(i+1) + " and " + str(j+1) +  " have common factor: ")
            print (n)


