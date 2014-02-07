require 'openssl'

puts "RSA Test"

public_key = File.read('public-key.pem')
plain_data = 'random seed'

key = OpenSSL::PKey::RSA.new(public_key)
encrypted_data = key.public_encrypt(plain_data)
puts encrypted_data

private_key = File.read('private-key.pem')
password = 'dumb'

deckey = OpenSSL::PKey::RSA.new(private_key, password)
m = deckey.private_decrypt(encrypted_data)
puts m