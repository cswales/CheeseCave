# Server code for the CheeseCave
import BaseHTTPServer
import cgi
import json
import subprocess
import sys

class CheesecaveHandler(BaseHTTPServer.BaseHTTPRequestHandler):

	DOC_ROOT = "/opt/pi/CheeseCave/DocRoot"
	SNAPSHOT = "/opt/pi/CheeseCave/current_snapshot"

#	def do_HEAD(self):
#		if self.path == "/":
#			self.send_header("Content-Type", "application/json")
#			self.end_headers()
#		elif self.path == "/stories":
#			self.send_header("Content-Type", "application/json")
#			self.end_headers()
#		else:
#			pass
#				
	def do_GET(self):
		# all API functions start with '/cheesecave'. If it doesn't 
		# start with 'cheesecave', serve up a straight up document from
		# the document root.
		print self.path
		print ("Doing get...")
		sys.stdout.flush()	
		if self.path.startswith("/cheesecave/"):
			print "API function"
			api_objs = self.path.split('?')[0].split('/')
			attr = api_objs[2]
			print attr
			sys.stdout.flush()
			if attr == "temp" or attr == "desired_temp" or attr == "humidity" or attr == "desired_humidity" or attr == "epoch":
				print "know attr"
				if attr == "temp":
					returnStr = getTemperatureJSON()
					# what about getting historical tempuratures?
				elif attr == "desired_temp":
					returnStr = getDesiredTempJSON()
				elif attr == "humidity":
					returnStr = getHumidityJSON()
				elif attr == "desired_humidity":
					returnStr = getDesiredHumidityJSON()
				elif attr == "epoch":
					returnStr = getEpochJSON();
				self.send_response(200)
				self.send_header("Content-Type", "application/json")
				self.end_headers()
				self.wfile.write(returnStr)
			elif attr == "snapshot":
				with open(self.SNAPSHOT, "rb") as f:
					self.send_response(200)
					self.send_header("Content_Type", "image/jpeg")
					self.end_headers()
					self.wfile.write(f.read())
					
			else: # or catch throw from others...
				self.send_response(404)	
				
		else:
			print "attempting to send file" 
			self.send_file()

	def do_POST(self):
		print("doing PoST")
		if self.path.startswith("/cheesecave/"):
			api_objs = self.path.split('/')	
			if (self.path == "/cheesecave/"):
				postVars = self.parse_POST()
				if ("desired_humidity" in postVars): 
					setDesiredHumidity(int(postVars["desired_humidity"][0]))
				if "desired_temperature" in postVars:
					setDesiredTemperature(int(postVars["desired_temperature"][0]))
				self.send_response(200)
			elif api_objs[2] == "snapshot":
				takeSnapshot() # for the moment, we'll make this synchronous. Bad us.
				#with open(self.SNAPSHOT, "rb") as f:
				self.send_response(200)
				#self.end_headers()
				#self.wfile.write(f.read())
			else:
				self.send_response(503) # is this right? I don't think so
		else:
			self.send_response(404)

	def parse_POST(self):
		ctype, pdict = cgi.parse_header(self.headers['Content-Type'])
		if (ctype == "multipart/form-data"):
			postvars = cgi.parse_multipart(self.rfile, pdict)
		elif (ctype == "application/x-www-form-urlencoded"):
			length = int(self.headers.getheader('Content-Length'))
			postvars = cgi.parse_qs(self.rfile.read(length), keep_blank_values=1)
		else:
			postvars = {}

		return postvars

	def send_file(self):
		if self.path == "" or self.path == "/":
			self.path = "/index.html"
		full_path = self.DOC_ROOT + self.path
		print full_path
		try: 
			with open(full_path, "rb") as f: # catch throws here!!
				self.send_response(200)
				self.send_header("Content-Type", getContentType(self.path))
				self.end_headers()
				self.wfile.write(f.read())
		except IOError:
			self.send_response(404)

CONFIG_FILE = "/opt/pi/CheeseCave/config.json"
STATE_FILE = "/var/lib/CheeseCave/sensor1-state.json"
SNAPSHOT_EXECUTABLE = "/opt/pi/CheeseCave/snapshot.sh"

def takeSnapshot():
	print "attempting to call snapshot.sh"
	subprocess.Popen(["sh", SNAPSHOT_EXECUTABLE])
	print "finished calling snapshot.sh"
	sys.stdout.flush()

def getTemperatureJSON():
	with open(STATE_FILE, 'r') as file:
		data = json.load(file)
		return json.dumps([data["temperature"]])
		#return "[52]"

def getHumidityJSON():
	with  open(STATE_FILE, 'r') as file:
		data = json.load(file)
		return json.dumps([data["humidity"]])
		#return "[88]"

def getEpochJSON():
	with open(STATE_FILE, 'r') as file:
		data = json.load(file)
		return json.dumps([data["epoch"]]);

def getDesiredTempJSON():
	with open(CONFIG_FILE, 'r') as file:
		data = json.load(file)
		return json.dumps([data["targets"]["temperature"]])
		#return "[55]"

def setDesiredTemperature(temp):
	with open(CONFIG_FILE, 'r+') as file:
		print "set desired temperature - start"
		data = json.load(file)
		print "loaded json file: ",data
		data["targets"]["temperature"] = temp
		print "set new value"
		file.seek(0)
		file.truncate()
		json.dump(data,file)
		print "wrote file"
		file.close()

def setDesiredHumidity(humidity):
	with open(CONFIG_FILE, 'r+') as file:
		data = json.load(file)
		print "set desired humidity - start"
		data["targets"]["humidity"] = humidity
		print "loaded json file: ",data
		file.seek(0)
		file.truncate()
		json.dump(data,file)
		print "wrote file"
		file.close()
		

def getDesiredHumidityJSON():
	with open(CONFIG_FILE, 'r') as file:
		data = json.load(file)
		return json.dumps([data["targets"]["humidity"]])
		#return"[89]"


def getContentType(path):
	if path.endswith(".jpeg"):
		return "img/jpeg"
	elif path.endswith(".png"):
		return "img/png"
	elif path.endswith(".gif"):
		return "img/gif"
	elif path.endswith(".js"):
		return "text/javascript"
	elif path.endswith(".html"):
		return "text/html"
	elif path.endswith(".css"):
		return "text/css"

	return "text/plain"
		
if __name__ == '__main__':
	server = BaseHTTPServer.HTTPServer(('0.0.0.0', 9000), CheesecaveHandler)
	try:
		server.serve_forever()
	except KeyboardInterrupt:
		pass
	server.server_close()
		
	
	
