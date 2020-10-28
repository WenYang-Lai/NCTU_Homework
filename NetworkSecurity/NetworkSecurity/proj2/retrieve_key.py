#!/usr/bin/python

from Crypto.PublicKey import RSA
from fractions import gcd
import gmpy

targets = ['pems/pubkey_3.pem', 'pems/pubkey_8.pem']

pems = []

for target in targets:
    p = open(target).read()
    pems.append(RSA.importKey(p))

print ("Get n1:")
print (pems[0].n)

p = gcd(pems[0].n, pems[1].n)
print ("Get p: ")
print (p)

q1 = pems[0].n // p
print ("Get q1: ")
print (q1)

d1 = long(gmpy.invert(pems[0].e, (p-1) * (q1-1)) )
print ("Private key d1")
print (d1)

private_key = RSA.construct((pems[0].n, pems[0].e, d1, p, q1))
open("private_key_3.pem", 'w').write(private_key.exportKey('PEM'))

