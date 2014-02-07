from sage.all import *

import sys
import random

p = next_prime(int(sys.argv[1]))
g = random.randint(2, p-1) # choose random generator
x1 = random.randint(0, p-1) # random message
x2 = random.randint(0, p-1)
print p, g, x1, x2

# setup
h1 = (g**x1) % p
h2 = (g**x2) % p
print h1, h2

# original message seed
m = random.randint(0, p-1)

# encrypt first layer..
m1 = m
y1 = random.randint(0, p-1)
c11 = (g**y1) % p
c21 = (m1 * (h1**y1)) % p
print c11, c21

# encrypt second layer...
m2 = (c11 + h2) % p
y2 = random.randint(0, p-1)
c12 = (g**y2) % p
c22 = (m2 * (h2 ** y2)) % p
print c12, c22

# c = (c12, c22) is what's actually sent over the wire

# decrypt outer layer
s2 = (c12 ** x2) % p
m2 = (c22 * (inverse_mod(s2, p))) % p
c11n = (m2 - h2) % p

print m2 == ((c11 + h2) % p)
print(c11n == c11)


# decrypt inner layer
s1 = (c11n**x1) % p
m1 = (m2 * (inverse_mod(s1, p))) % p
print(m1)

print(m == m1)