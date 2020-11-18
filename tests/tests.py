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

def test_post(port: int) -> str:
	req = requests.post(get_base_url(port) + "post/a", data="hello world")
	if (req.status_code != 201):
		return "Bad status code."
	req = requests.get(get_base_url(port) + "post/a")
	if (req.status_code != 200):
		return "File hasn't been created."
	if (req.text != "hello world"):
		return "Bad content in the file."
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

def test_two_posts(port: int) -> str:
	req = requests.post(get_base_url(port) + "post/b", data="1")
	if (req.status_code != 201):
		return "Bad status code on first creation."
	req = requests.post(get_base_url(port) + "post/b", data="2")
	if (req.status_code != 204):
		return "Bad status code on update."
	return ""

def test_two_puts(port: int) -> str:
	req = requests.put(get_base_url(port) + "post/c", data="1")
	if (req.status_code != 201):
		return "Bad status code on first creation."
	req = requests.put(get_base_url(port) + "post/c", data="2")
	if (req.status_code != 204):
		return "Bad status code on update."
	return ""