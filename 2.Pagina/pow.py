#POW EXAMPLE

from hashlib import sha256
#Let’s decide that the hash of some integer x multiplied by another y must end in 0
x = 5
y = 0  # We don't know what y should be yet...
while sha256(f'{x*y}'.encode()).hexdigest()[-1] != "0":
    y += 1
print(f'The solution is y = {y}')
#hash(5 * 21) = 1253e9373e...5e3600155e860

