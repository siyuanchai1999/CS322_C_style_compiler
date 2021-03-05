#!/usr/bin/env python3

# 28-bit fixed point integer
L = 28
M = 2**L

ROUND = 40

def rep(N):
  return 1.0 * N / M

sum = 0
for i in range(100000):
  b = 2*i + 1

  shift = 0
  quot = b
  while quot > 0:
    shift = shift + 1
    quot = quot >> 1

  x = M >> shift
  B = b << L
  for j in range(ROUND):
    x = (x * (2*M - ((B * x) >> L))) >> L
  # print("compute 1 /", b, ", res =", rep(x))

  if (i & 1):
    sum = sum - x*4
  else:
    sum = sum + x*4

print(rep(sum))

W=10000000000000000//M
print("float ", W, sum)
print("answer", W*sum)
print("bounds", 2**62)
