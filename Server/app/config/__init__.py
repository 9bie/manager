from .config import CONFIG
import random,string
if CONFIG["web"]["control"] == "/pleasechangeme/":
	WEB = ''.join(random.sample(string.ascii_letters + string.digits, 8)) 
	CONFIG["web"]["control"] = "/%s/" % ( WEB )
	print("!!!!!!!!!!!!!!\nPlease Change app/config/config.py' s CONFIG['WEB']['control'] for your control gui\nYou temporary web control page is: /%s/\n!!!!!!!!!!!!!!" % WEB )