"""
Flask WEB Server. Flask Packages to start with.

"""
from flask import Flask, request, url_for, render_template
from flask import session
from flask import abort, redirect, flash, jsonify
import sys
from collections import defaultdict

# Import from supporting file
from domain import *
import json

# LDAp3 #
from ldap3 import Server, ALL, Connection


# FLASK-LOGIN IMPORTS
from flask_ldap3_login import LDAP3LoginManager
from flask_login import LoginManager, UserMixin, current_user
from flask_login import login_required, logout_user, login_user
from flask import render_template_string
from flask_ldap3_login.forms import LDAPLoginForm

# Python Logger
import logging
log = logging.getLogger(__name__)


# Instance of Flask APP
app = Flask(__name__)



# GLOBALS... #
global LDAP_HOST_NAME
global LDAP_PORT_NAME
global LDAP_BASE_DN_NAME

# Global Var's for Flask APP
global my_env
global fields
global DISP_FLAG
global ERROR_MSG
global SVN_MSG


# Setting up configuration variables
app.config['LDAP_HOST'] = 'ipa-idm-east-1b.aws.cccis.com'
app.config['LDAP_PORT'] = 389
app.config['SECRET_KEY'] = 'my precious secret'
app.config['LDAP_BASE_DN'] = 'cn=users,cn=accounts,dc=aws,dc=cccis,dc=com'	# Base directory into LDAP

# This is used to search into LDAP by UID=user with valid access (don't have to include in "uid=schanged")
app.config['LDAP_BIND_USER_DN'] = 'cn=users,cn=accounts,dc=aws,dc=cccis,dc=com'


# Initiate login_manager & ldap_managers
try:
	# Create Login_Manager
	login_manager = LoginManager(app)
	login_manager.init_app(app)

except:
	print("FLASK: Login Manager Failed: Exiting Flask")
	sys.exit(0)

# Initialise LDAP_MANAGER
try:
	# LDAP Manager Init.
	ldap_manager = LDAP3LoginManager(app)
except:
	print("FLASK: LDAP Manager Auth Failed: Exiting Flask")
	sys.exit(0)


"""
USER CLASS:
manual USER class to initiate application specific methods and decorators
Return: User logged-in information

"""

class User(UserMixin):

	def __init__(self, dn, username, info):
		self.dn = dn
		self.username = username
		self.info = info
	
	""" This affects the save_user loader function... check for return values"""
	# Returns User-ID instead of self.dn
	def __repr__(self):
		return self.username
	
	""" This affects the load_user function... check return to self.dn"""
	def get_id(self):
		return self.username
	
	def take_id(self,dn):
		return self.username
	
	@property
	def is_active(self):
		return True
	
	@property
	def is_anonymous(self):
		return False



"""
SAMPLE RAW PAGES ARE HERE....
Test for Authorized access
"""

# RAW-1
@app.route('/raw2',methods=['GET','POST'])
#@login_required
def raw2():
	template = """
		<h1>GIrl has No Name</h1>
		<h2>Game of Thrones !</h2>
		"""
	return render_template_string(template)

# RAW-2
@app.route('/raw1',methods=['GET','POST'])
#@login_required
def raw1():
	template = """
		<h1>Winter IS Coming</h1>
		<h2>YOu know Nothing Snow !</h2>
		"""
	return render_template_string(template)


"""
USER_LOADER: Load user back from Session.
Validates user is Logged In and Session does not expire.
"""

@login_manager.user_loader
def load_user(id):

	global LDAP_BASE_DN_NAME
	global LDAP_HOST_NAME

	# GLOBALS... #
	LDAP_HOST_NAME = 'ipa-idm-east-1b.aws.cccis.com'
	LDAP_PORT_NAME = 389
	LDAP_BASE_DN_NAME = 'cn=users,cn=accounts,dc=aws,dc=cccis,dc=com'
	 
	if id == session['username']:
				
		my_server = Server(LDAP_HOST_NAME, get_info=ALL)
		# Using anonymous BIND
		
		#ldap_bind_user_dn = "uid="+session['username']+","+LDAP_BASE_DN_NAME
		#print(ldap_bind_user_dn)
		
		done = 0
		while(done < 5):
			try:
				my_conn = Connection(my_server)
				#my_conn = Connection(my_server,user=ldap_bind_user_dn, password=passwd_hash)
				done = 5
			except:
				print("LDAP SOcket connection ERROR...Trying Attempt:%s" %done)
				done = done + 1
				if done == 5:
					return None
				continue

		done = 0
		while(done < 5):
			try:
				# First BIND it...
				my_conn.bind()
				done = 5
			except:
				print("LDAP-BIND Socket Error. Trying Attempt: %s" %done)
				done = done + 1
				if done == 5:
					return None
				continue

		# Search using Anonymous BIND
		filter_name = "(uid="+session['username']+")"
		check = my_conn.search(LDAP_BASE_DN_NAME, filter_name)
		
		# Unbind After USE...
		my_conn.unbind()
		if check == True:
			return User('',session['username'],'')
		else:
			print("LDAP User Does not Exists...")
			return None
	"""If user not exists in session, just make it anonymous user... """
	return None


"""
Declare The User Saver for Flask-Ldap3-Login
This method is called whenever a LDAPLoginForm() successfully validates.
Here you have to save the user, and return it so it can be used in the
login controller.

Save_user: USer Logged-in saved to Sessions.
"""

@ldap_manager.save_user
def save_user(dn, username, info, memberships):
	user = User(dn,username,info)
	session['username'] = str(user)
	#print("into save user:%s" %session['username'])
	#print(type(session['username']))
	#TODO send this to function to get Groups
	session['info'] = str(user.info)
	return user


"""
FIRST PAGE TO START WITH...

"""
@app.route('/', methods=['GET','POST'])
def home():

	if current_user is None or current_user.is_anonymous:
		return redirect(url_for('login'))
	# User Logged IN... GOTO index page
	return redirect(url_for('index'))


"""
LOGIN: check for USer LDAP Login and validations.
User creadentials are checked against LDAP.

"""
@app.route('/login', methods=["GET","POST"])
def login():

	if request.method == 'GET':
		
		"""Instantiate a LDAPLoginForm which has a validator to check if the user exists in LDAP."""
		form = LDAPLoginForm()

	elif request.method == 'POST':
		form = LDAPLoginForm(request.form)
		session['password'] = str(form.password.data)
		# Validate
		try:
			if form.validate_ldap():
				# Flask Login : Please check
				login_user(form.user)
			return redirect('/')
		except:
			global ERROR_MSG
			ERROR_MSG = "Invalid Username/Password. You Don't Exists ! Go Away. "
			return render_template("login.html", form=form, ERROR_MSG=ERROR_MSG)

	return render_template("login.html", form=form)



"""
INDEX Page: Fresh page display.

"""
@app.route('/index', methods=["GET", "POST"])
@login_required
def index():
	
	# Get Globals to Work
	global DISP_FLAG
	global my_env
	
	# POST HTTP Method
	if request.method == 'POST':
		# {<select name="">, <option value=""> }
		env = request.form['query']
		my_env = str(env)
		# Menu...
		env_list = Domain("temp").get_env_var()
		# Get properties for selected ENV.
		fields = Domain(env).get_env_data()
		# Display Table of properties.
		if len(fields):
			DISP_FLAG = 1
		
		"""
		# fields: Returned values dict from environment_name
		# env_list: environments from file
		# my_env: selected environment variable
		# DISP_FLAG: to display table
		"""
		return render_template(
			'envForm.html',
			fields=fields,
			env_list=env_list,
			my_env=my_env,
			user_name=session['username'],
			DISP_FLAG=DISP_FLAG
		)
	
	# GET HTTP Method
	elif request.method == 'GET':
		DISP_FLAG = 0
		try:
			if session['username']:
				env_list = Domain("temp").get_env_var()
				return render_template(
					'envForm.html',
					env_list=env_list,
					user_name=session['username'],
					DISP_FLAG=DISP_FLAG
				)
		except:
			form = LDAPLoginForm()
			session.clear()
			ERROR_MSG = "Sesison Expired. Retry Again."
			return render_template('login.html', form=form, ERROR_MSG=ERROR_MSG)


"""
Update Page: Make actual changes to File

"""
@app.route('/update', methods=["GET", "POST"])
@login_required
def update():

	# Put Globals to Work
	global fields
	global my_env
	global DISP_FLAG
	global SVN_MSG
	global ERROR_MSG
	
	env_list = Domain("temp").get_env_var()
	
	if request.method == 'POST':
		update_list = request.json
		print("UPDATE POST now...")
		fields, svn_diff_flag = Domain(my_env).update_xml_file(my_env,update_list)
		print(fields)
		SVN_MSG = svn_diff_flag
		return render_template(
			'envForm.html',
			SVN_MSG=SVN_MSG,
			fields=fields,
			env_list=env_list,
			my_env=my_env,
			user_name=session['username'],
			DISP_FLAG=DISP_FLAG
		)
	elif request.method == 'GET':
		print("UPDATE GET METHOD")
		DISP_FLAG = 1
		try:
			# All According to Plan.
			print("UPDATE GET: %s" %fields)
			return render_template(
				'envForm.html',
				SVN_MSG=SVN_MSG,
				env_list=env_list,
				fields=fields,
				my_env=my_env,
				user_name=session['username'],
				DISP_FLAG=DISP_FLAG
			)
		except Exception as e:
			# If fields is invalid here...
			try:
				# Used if Username is still valid even after invalid-request.
				print("UPDATE GET: TRY METHOD")
				DISP_FLAG = 0
				return render_template(
					'envForm.html',
					env_list=env_list,
					user_name=session['username'],
					DISP_FLAG=DISP_FLAG
				)
			except:
				session.clear()
				ERROR_MSG = "Session Expired. You are too Slow. But steady wins the RACE. Login again."
		
		form = LDAPLoginForm()
		session.clear()
		ERROR_MSG = "Sesison Expired. Please login Again."
		return render_template('login.html', form=form, ERROR_MSG=ERROR_MSG)


"""
COMMIT Page: Display Commit Page
"""
@app.route('/commit', methods=["GET","POST"])
@login_required
def commit():
    global my_env
    global fields
    global SVN_MSG

    if request.method == "POST":
        global fields
        cmt_mesg = request.json
        vals = commit_to_server(str(cmt_mesg))
        if vals == True:
            env_list = Domain("temp").get_env_var()
            SVN_MSG = "Commited Changes..."
            return render_template(
                'envForm.html',
                fields=fields,
                user_name=session['username'],
                env_list=env_list,
                SVN_MSG=SVN_MSG
            )
        elif vals == False:
        	env_list = Domain("temp").get_env_var()
        	ERROR_MSG = "BAD with Update: Kindly Make Update Again."
        	return render_template(
                'envForm.html',
                fields=fields,
                user_name=session['username'],
                env_list=env_list,
                ERROR_MSG=ERROR_MSG
            )
    elif request.method == "GET":
        env_list = Domain("temp").get_env_var()
        #SVN_MSG = "Commited Changes..."
        DISP_FLAG = 0
        try:
            return render_template(
                'envForm.html',
                fields=fields,
                user_name=session['username'],
                env_list=env_list,
                SVN_MSG=SVN_MSG,
                DISP_FLAG=DISP_FLAG
            )
        except:
            return render_template('envForm.html',user_name=session['username'],env_list=env_list, DISP_FLAG=DISP_FLAG)



""" LOGOUT Page """

@app.route("/logout")
@login_required
def logout():
	
	session.clear()
	logout_user()
	return redirect('/')


if __name__ == '__main__':
	app.run(debug=True)
