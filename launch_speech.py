import base64
import socket
import os
import hashlib
import hmac
import requests
import time
import uuid
from urllib import parse
import http.client
import urllib.parse
import json

def encode_text(text):
	encoded_text = parse.quote_plus(text)
	return encoded_text.replace('+', '%20').replace('*', '%2A').replace('%7E', '~')

def encode_dict(dic):
	keys = dic.keys()
	dic_sorted = [(key, dic[key]) for key in sorted(keys)]
	encoded_text = parse.urlencode(dic_sorted)
	return encoded_text.replace('+', '%20').replace('*', '%2A').replace('%7E', '~')

parameters = {'AccessKeyId': os.environ['access_key_id'],
			  'Action': 'CreateToken',
			  'Format': 'JSON',
			  'RegionId': 'cn-shanghai',
			  'SignatureMethod': 'HMAC-SHA1',
			  'SignatureNonce': str(uuid.uuid1()),
			  'SignatureVersion': '1.0',
			  'Timestamp': time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
			  'Version': '2019-02-28'}
query_string = encode_dict(parameters)
string_to_sign = 'GET' + '&' + encode_text('/') + '&' + encode_text(query_string)
secreted_string = hmac.new(bytes(os.environ['access_key_secret'] + '&', encoding='utf-8'), bytes(string_to_sign, encoding='utf-8'), hashlib.sha1).digest()
signature = encode_text(base64.b64encode(secreted_string))
full_url = 'http://nls-meta.cn-shanghai.aliyuncs.com/?Signature=%s&%s' % (signature, query_string)
response = requests.get(full_url)
if response.ok:
	root_obj = response.json()
	key = 'Token'
	if key in root_obj:
		token = root_obj[key]['Id']
		expire_time = root_obj[key]['ExpireTime']
		os.environ['token'] = token
		print(os.environ['token'])
		os.execl("speech", "speech")
