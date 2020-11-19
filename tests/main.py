"""
The run_test function is a wrapper for your test.
Your test should be a function that takes a port as an int, and returns a string, empty if the test is
successfull, containing the reason of the failure if failed.
"""
from tests import *
from typing import Callable
import sys
import os

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[39m"

def cleanup() -> None:
	"""
		Remove file created by the tester to make sure the test is new every run.
	"""
	os.system("rm -rf ./tests/www/tmp/*")

def run_test(test_name: str, test: Callable, port: int) -> None:
	"""
		Runs a test defined in function test, with the name test_name,
		and prints wether it passed or not.
	"""
	try:
		result = test(port)
	except:
		result = "Cannot connect to the sever."
	char = ""
	color = GREEN
	if (len(result) == 0):
		char = "✅"
	else:
		color = RED
		char = "❌"
	print(r"{:40} {}{} {}{}".format(test_name, color, char, result, RESET))

def run(port: int) -> None:
	"""
		Entrypoint of the tester
	"""
	run_test("GET /", test_get, port)
	run_test("GET /a/", test_get_dir_index, port)
	run_test("POST /", test_wrong_post, port)
	run_test("GET /virtual/a/", test_get_dir_not_allowed, port)
	run_test("POST /post/test too big payload", test_too_big_request, port)
	run_test("GET /b, checking custom 404 page", test_custom_404, port)
	run_test("GET /doidjo...", test_404, port)
	run_test("GET /index/a/", test_autoindex, port)
	run_test("PUT /post/c * 2", test_two_puts, port)

if (__name__ == "__main__"):
	if (len(sys.argv) != 2):
		print("Usage: python3 main.py <port number>")
		exit(1)
	try:
		port = int(sys.argv[1])
	except:
		print("Invalid port")
		exit(1)
	cleanup()
	run(port)