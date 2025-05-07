import requests
import json
import hmac
import hashlib
import time
import paho.mqtt.client as mqtt

def sign(key, msg):
    return hmac.new(key, msg.encode('utf-8'), hashlib.sha256).digest()

def getSignatureKey(key, dateStamp, regionName, serviceName):
    kDate = sign(('AWS4' + key).encode('utf-8'), dateStamp)
    kRegion = sign(kDate, regionName)
    kService = sign(kRegion, serviceName)
    kSigning = sign(kService, 'aws4_request')
    return kSigning

def update_dynamodb(room_number, is_occupied):
    access_key = 'AKIAXKPUZQEXHPC64HBJ'
    secret_key = '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'
    region = 'us-east-1'
    service = 'dynamodb'
    host = 'dynamodb.us-east-1.amazonaws.com'
    endpoint = 'https://' + host + '/'
    amz_target = 'DynamoDB_20120810.PutItem'
    content_type = 'application/x-amz-json-1.0'
    request_parameters = json.dumps({
        "TableName": "Room_Occupancy_Table",
        "Item": {
            "room_number": {"N": str(room_number)},
            "is_occupied": {"BOOL": is_occupied}
        }
    })

    t = time.gmtime()
    amz_date = time.strftime('%Y%m%dT%H%M%SZ', t)
    date_stamp = time.strftime('%Y%m%d', t)

    canonical_uri = '/'
    canonical_querystring = ''
    canonical_headers = 'content-type:' + content_type + '\n' + 'host:' + host + '\n' + 'x-amz-date:' + amz_date + '\n' + 'x-amz-target:' + amz_target + '\n'
    signed_headers = 'content-type;host;x-amz-date;x-amz-target'
    payload_hash = hashlib.sha256(request_parameters.encode('utf-8')).hexdigest()
    canonical_request = 'POST\n' + canonical_uri + '\n' + canonical_querystring + '\n' + canonical_headers + '\n' + signed_headers + '\n' + payload_hash

    algorithm = 'AWS4-HMAC-SHA256'
    credential_scope = date_stamp + '/' + region + '/' + service + '/' + 'aws4_request'
    string_to_sign = algorithm + '\n' + amz_date + '\n' + credential_scope + '\n' + hashlib.sha256(canonical_request.encode('utf-8')).hexdigest()

    signing_key = getSignatureKey(secret_key, date_stamp, region, service)
    signature = hmac.new(signing_key, string_to_sign.encode('utf-8'), hashlib.sha256).hexdigest()

    authorization_header = algorithm + ' ' + 'Credential=' + access_key + '/' + credential_scope + ', ' + 'SignedHeaders=' + signed_headers + ', ' + 'Signature=' + signature

    headers = {
        'Content-Type': content_type,
        'X-Amz-Date': amz_date,
        'X-Amz-Target': amz_target,
        'Authorization': authorization_header
    }

    response = requests.post(endpoint, headers=headers, data=request_parameters)
    print("PutItem succeeded:", response.text)

# Example usage
update_dynamodb(10001, False)