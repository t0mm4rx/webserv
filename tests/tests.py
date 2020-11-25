import requests

def get_base_url(port: int) -> str:
	"""
		Return the base URL of the server
	"""
	return "http://localhost:{}/".format(port)

def test_get(port: int) -> str:
	req = requests.get(get_base_url(port))
	if (req.status_code != 200):
		return "Bad status code."
	if (req.text != "hello world"):
		return "Bad content."
	if (req.headers['Content-Length'] != "11"):
		return "Bad Content-Length"
	if (req.headers['Content-Type'] != "text/html"):
		return "Bad Content-Length"
	return ""

def test_get_dir_index(port: int) -> str:
	req = requests.get(get_base_url(port) + "a")
	if (req.status_code != 200):
		return "Bad status code."
	if (req.text != "hello"):
		return "Bad content."
	return ""

def test_get_dir_not_allowed(port: int) -> str:
	req = requests.get(get_base_url(port) + "virtual/a/")
	if (req.status_code != 403):
		return "Bad status code."
	return ""

def test_wrong_post(port: int) -> str:
	req = requests.post(get_base_url(port))
	if (req.status_code != 405):
		return "Bad status code."
	return ""

def test_too_big_request(port: int) -> str:
	payload = "a" * 1025
	req = requests.post(get_base_url(port) + "post/test", data=payload)
	if (req.status_code != 413):
		return "Bad status code."
	return ""

def test_custom_404(port: int) -> str:
	req = requests.get(get_base_url(port) + "b")
	if (req.status_code != 404):
		return "Bad status code"
	if (req.text != "custom404"):
		return "Bad error page"
	return ""

def test_404(port: int) -> str:
	req = requests.get(get_base_url(port) + "doidjodoeijdosejfoejfseoifjseiofjsejfsejfesjfseofsejiseofj")
	if (req.status_code != 404):
		return "Bad status code"
	return ""

def test_autoindex(port: int) -> str:
	req = requests.get(get_base_url(port) + "index/a/")
	if (req.status_code != 200):
		return "Bad status code"
	return ""

def test_two_puts(port: int) -> str:
	req = requests.put(get_base_url(port) + "post/c", data="1")
	if (req.status_code != 201):
		return "Bad status code on first creation."
	req = requests.put(get_base_url(port) + "post/c", data="2")
	if (req.status_code != 204):
		return "Bad status code on update."
	return ""

def test_multiple_ports(port: int) -> str:
	req = requests.get(get_base_url(port))
	if (req.text != "hello world"):
		return "Bad content on first port."
	req = requests.get(get_base_url(port + 1))
	if (req.text != "second file"):
		return "Bad content on second port."
	return ""

def test_different_index(port: int) -> str:
	req = requests.get(get_base_url(port))
	if (req.text != "hello world"):
		return "Bad first index."
	req = requests.get(get_base_url(port) + "2/")
	if (req.text != "second file"):
		return "Bad second index."
	return ""

def test_head(port: int) -> str:
	req = requests.head(get_base_url(port))
	if (req.status_code != 200):
		return "Bad status code."
	if (len(req.text) > 0):
		return "Head returned some content."
	return ""

def test_trace(port: int) -> str:
	req = requests.request('TRACE', get_base_url(port))
	if (req.status_code != 200):
		return "Wrong status code."
	if (len(req.text.split("\n")[0].split()) != 3):
		return "Wrong response."
	return ""

def test_multiple_get(port: int) -> str:
	for i in range(100):
		req = requests.get(get_base_url(port))
		if (req.status_code != 200 or req.text != "hello world"):
			return "Bad request at {}th iteration.".format(i + 1)
	return ""

def test_delete(port: int) -> str:
	requests.put(get_base_url(port) + "post/test", data="test put and delete")
	req = requests.get(get_base_url(port) + "post/test")
	if (req.status_code != 200 and req.text != "test put and delete"):
		return "Bad put request."
	req = requests.delete(get_base_url(port) + "post/test")
	if (req.status_code != 202 and req.status_code != 200):
		return "Bad status code for DELETE."
	req = requests.get(get_base_url(port) + "post/test")
	if (req.status_code != 404):
		return "File still exists after DELETE."
	return ""

def test_connect(port: int) -> str:
	req = requests.request('CONNECT', get_base_url(port))
	if (req.status_code != 200):
		return "Bad status code."
	return ""
# CGI
# PUT avec chunk
# Multiple GET
# CONNECT