

class AWSCred(object):
    """
    Stores Amazon Web Service credentials. These are used in all requests with AWS-compliant services.
    See: http://docs.amazonwebservices.com/AmazonS3/latest/dev/index.html?S3_Authentication.html
    """
    
    def __init__(self, name, aws_access_key_id, aws_secret_access_key):
        
        assert isinstance(aws_access_key_id, basestring)
        assert isinstance(aws_secret_access_key, basestring)
        self.name = name
        self.access_key_id = str(aws_access_key_id)
        self.secret_access_key = str(aws_secret_access_key)
        
    def __str__(self):
        return "AWSCred: \n\tkey_id:%s\nacces_key:\t%s" % (self.access_key_id, "HIDDEN_SECRET_KEY")