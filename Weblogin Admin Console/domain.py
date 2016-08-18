from collections import defaultdict
import subprocess
import os, shutil, fcntl
import sys
import re
from bs4 import BeautifulSoup, Comment
from app import logger_ui

SVN_URL = "http://svn.proddev.cccis.com:8090/svn/build-support/trunk/test-sunny/"

TEMP_FILE_DIR = "/root/Desktop/mywork"
SVN_DIR = TEMP_FILE_DIR + "/test-sunny"
TEMP_FILE = TEMP_FILE_DIR + "/temp.xml"


# Dir containing settings.xml file...
#SVN_DIR = "/Users/ocervell/Drive/workspace/ccc/svn/build-support/branches/development-ocervell/test-dir/source"
#TEMP_FILE_DIR = "/Users/ocervell/Drive/workspace/ccc/svn/build-support/branches/development-ocervell/test-dir"

# Temp File When Update is clicked...
#temp_file = SVN_DIR + "/temp.xml"


SVN_DIFF_MSG = {
  "SVN_UPDATE_SUCCESS":"SVN Update Success.",
  "SVN DIFF_FAIL":"SVN Diff failed. unable to apply Conflicts.",
  "SVN_FILE_RENAME":"SVN FIle renamed Success.",
  "SVN_CONFLICT_PULL":"SVN Conflict detected and Pulled Successfully. Please Update Again.",
  "SVN_NO_CONFLICT":"No SVN Conflict detected. Your Update is Successful.",
  "SVN_FINISH":"Success ! Changes Saved.",
  "SVN_DETAILS":"Remember: UPDATE first before making any change & then COMMIT.",
  "SVN_COMMIT_SUCCESS":"SVN Commit Success. Changes Commited."
}

env_update_dict = {}

# Only used in update process...
class updates:
  globs_dict = {}



"""
Commit Method: to be called with commit message.
Return: 1. unchanged: if <update> is not done
        2. modified: if update is clicked
"""
def commit_to_server(mesg):

  cmt_msg = str(mesg)
  logger_ui.info("Commit MESG:%s" %cmt_msg)

  # UPDATE Button clicked
  if os.path.isfile(TEMP_FILE_DIR + "/temp.xml"):
    ret = os.system('mv %s %s' % (TEMP_FILE_DIR + "/temp.xml", SVN_DIR + '/settings.xml'))
    logger_ui.info("COMMIT: Settings.xml file updated from temp.xml. Return code: %s" % ret)
  else:
    # No Update processed yet...
    logger_ui.info("COMMIT: No changes made to File.")
    return False

  # Execute SVN Shell Commands.
  try:
    p = subprocess.Popen(['svn','update'], cwd=SVN_DIR)
    p.wait()
  except:
    logger_ui.info("COMMIT: SVN update failed...")
    return False

  logger_ui.info("COMMIT: SVN update succeded...")

  # Execute SVN Commit
  #logger_ui.info("COMMIT: Initiating commit to dir: %s" % SVN_DIR)
  try:
    p = subprocess.Popen(['svn','commit','-m', cmt_msg], cwd=SVN_DIR)
    ret = p.communicate()[0]
    # p.wait()
  except Exception as e:
    logger_ui.info("COMMIT: SVN commit Failed. Check File status again.")
    logger_ui.exception(e)
    return False

  logger_ui.info("COMMIT: SVN commit succeded. Message: %s" % ret)

  # Execute SVN Shell Commands.
  try:
    p = subprocess.Popen(['svn','update'], cwd=SVN_DIR)
    p.wait()
  except:
    logger_ui.info("COMMIT: SVN update failed...")
    return False

  logger_ui.info("COMMIT: SVN update succeded...")

  # Update Global dict.
  updates.globs_dict = {}
  env_update_dict = {}
  return True


"""
Domain Class:
Args:
  1. Domain Name

"""
class Domain(object):

  """
  Init Class with class-variables.
  Args: dom: domain-id
  Params: domain_id, fname, temp_file, search_list, update_dict
  """
  def __init__(self, dom=None):

    # Environment name
    self.domain_id = "deployment."+dom.lower()
    self.temp_domain_id = "deployment."

    # settings.xml file path
    self.src_file = SVN_DIR + "/settings.xml"
    self.temp_file = TEMP_FILE_DIR + "/temp.xml"

    # To get Update status back
    self.search_list = []
    self.update_dict = {}

    if dom:
      self.soup = self.read_xml_file()
      self.env_dict = self.get_env_data(self.domain_id)
      self.envir_list = self.get_env_var()
    else:
      print("Usage: <domain.py> <environment_name>")
      sys.exit(0)
    #logger_ui.info("Domain.__init__() | Finish successfully")


  #-----------------------------------------------------#
  #Creates temporary updated file from User changes.    #
  #Args:                                                #
  #  global dict={"env-name":{'tag':'value'} }          #
  #param:                                               #
  #  Update final file with changes made                #
  #-----------------------------------------------------#
  def trial_create_update_file(self, env_update_dict):

    print("Trial-create-update;GLObe DICt:- %s" %updates.globs_dict)
    # Open temporary file in write-mode
    new_fp = open(self.temp_file,"w")

    # Read from Settings.xml
    try:
      old_fp = open(self.src_file, "r")
    except:
      #print("Update: settings.xml does not exists in path.")
      logger_ui.error("Settings.xml does not exists.")
      return False


    """
    Store number of environments Changed.
    Usage:
      my_env_lists = [ <id>dev0</id>, <id>dev1</id>... ]
      self.temp_domain_id = "deployment."
    used to compare with lines from settings.xml
    """

    my_env_lists = []
    for keys in env_update_dict.keys():
      my_env_lists.append("<id>"+self.temp_domain_id+keys+"</id>")


    # Precious: Do not Touch !
    # id_flag: <id> field match
    # update_done: Track all updated fields updated to file.
    # Found: when <id> <properties> match

    id_flag = False
    found = False
    update_done = False

    # Read from settings.xml and update to temp.xml
    for line in old_fp.readlines():
      """ Base conditions to satisfy """
      if update_done == False and (str(line).strip() in my_env_lists):
        # Updates not inserted & <id> does match
        id_flag = True
        my_env_name = str(line).strip()
      elif update_done == False and id_flag and str(line).strip() == "<properties>":
        # <id> match & <properties> found
        found = True

      """ Good to go with actual updates """
      if id_flag and found:
        # Both Condition match. Insert Starts.
        new_fp.write(line)
        """
        my_env_name = "<id>deployment.dev0</id>"
        temp_env = my_env_name[15:-5]
        """
        temp_env = str(my_env_name[15:-5])

        # get each environment from Updated Dict
        for keys, vals in env_update_dict[temp_env].items():
          if len(vals) > 0:
            result = "<"+keys+">"+str(vals)+"</"+keys+">"
          else:
            result = "<"+keys+">"
          new_fp.write(str(result)+'\n')

        """ Handle with care..."""
        update_done = True
        id_flag = False
        found = False
      elif update_done == True and str(line).strip() != "</properties>":
        # File Seek operation
        continue
      else:
        update_done = False
        new_fp.write(line)
    # Close all file descriptors.
    new_fp.close()
    old_fp.close()


  """
  Check for SVN Conflicts now...
  """
  def check_svn_conflicts(self, env_var, env_update_dict ):

    # 1. Check Conflicts. Note: shell output.
    try:
      proc = subprocess.Popen(['svn','diff','.',SVN_URL],cwd=SVN_DIR,stdout=subprocess.PIPE)
      output_list, error_list = proc.communicate()
      proc.wait()
    except Exception as e:
      print("Exception in SVN DIFF | %s | %s" %(type(e).__name__, str(e)))
      env_update_dict[env_var] = update_dict
      updates.globs_dict[env_var] = update_dict
      return SVN_DIFF_MSG['SVN_DIFF_FAIL']
      #return update_dict, SVN_DIFF_MSG['SVN_DIFF_FAIL']

    logger_ui.info("SVN Diff executed")
    # 2. Conflict detected.
    if output_list:
      try:
        proc2 = subprocess.Popen(['rm','-f','settings.xml'],cwd=SVN_DIR)
        proc2.wait()
      except Exception as e:
        logger_ui.error("Exception | %s | %s" % (type(e).__name__, str(e)))
        env_update_dict[env_var] = update_dict
        updates.globs_dict[env_var] = update_dict
        return SVN_DIFF_MSG['SVN_UPDATE_FAIL']
        #return update_dict, SVN_DIFF_MSG['SVN_UPDATE_FAIL']

      logger_ui.info("SVN | Removed settings.xml")
      # SVN Pull changes Now.
      try:
        proc3 = subprocess.Popen(['svn','update','.'], cwd=SVN_DIR)
        proc3.wait()
      except Exception as e:
        logger_ui.error("Exception | %s | %s" % (type(e).__name__, str(e)))
        env_update_dict[env_var] = update_dict
        updates.globs_dict[env_var] = update_dict
        return SVN_DIFF_MSG['SVN_UPDATE_FAIL']
        #return update_dict, SVN_DIFF_MSG['SVN_UPDATE_FAIL']

      logger_ui.info("SVN | Updated settings.xml")

      # Taste new Flavour of Soup. and Override changes.
      sample_dict = {}
      self.soup = self.read_xml_file()
      #print("ENV-update-dict is; %s" %env_update_dict)
      for env in env_update_dict.keys():
        #this gets old properties
        temp_dict = env_update_dict[env]
        #this gives new properties
        changed_dict = self.get_env_data(env)
        #print("Changed DICT for ENV-> %s is: %s" %(env,changed_dict))
        temp_dict.update(changed_dict)
        sample_dict[env] = temp_dict

      #print("Conflict zone: sample dict is: %s" %sample_dict)
      updates.globs_dict = sample_dict
      env_update_dict = sample_dict
      return SVN_DIFF_MSG['SVN_CONFLICT_PULL']
      
    # When no SVN conflicts detected.
    logger_ui.info("SVN | No conflicts.")
    #env_update_dict[env_var] = update_dict
    #print("NO Conflict zone: ENV dict is: %s" %env_update_dict)
    return SVN_DIFF_MSG['SVN_NO_CONFLICT']
    #return update_dict, SVN_DIFF_MSG['SVN_FINISH']

  """
  Update Button Click Returns Here.
  usage:
    1. check for SVN Conflict.
    2. then only update changes or prompt user for new update.
    3. If success: store changes to Temp.
    4. update global dict of changes
  """
  def update_xml_file(self, env_var, update_list):
    
    # per env update dict.
    update_dict = {}
    # Just for FUN... Hahaha
    for keys, vals in self.env_dict.items():
      self.search_list.append(keys)

    # Handle with care... Actual Update
    for data in update_list:
      txt = str(data)
      tag_name, tag_val = self.regex_tag_vals(txt)
      if len(tag_name) > 0 or len(tag_val) > 0:
        update_dict[str(tag_name)] = str(tag_val)

    logger_ui.info("Update Dict Created...")
    env_update_dict[env_var] = update_dict
    
    """USING NEW CLASS """
    updates.globs_dict[env_var] = update_dict
    #print(env_update_dict)
    # SVN Conflict Checks...
    svn_message = self.check_svn_conflicts(env_var, env_update_dict)
    #env_update_dict[env_var] = conflicted_dict
    # Create temp file to store changes...
    self.trial_create_update_file(env_update_dict)
    logger_ui.info("Created update file.")
    return env_update_dict[env_var], svn_message



  """
  Reading Settings.xml from SVN and create SOUP to
  traverse XML File.
  """
  def read_xml_file(self):

    # Get the data from SERVER...
    flag = False
    if os.path.isfile(SVN_DIR + "/settings.xml"):
      flag = True
    else:
      logger_ui.error("Settings.xml does not exists.")
      return None

    if flag:
      try:
        with open(self.src_file, "r+") as fp:
          sop = BeautifulSoup(fp,'xml')
      except Exception as e:
        logger_ui.error("BeautifulSoup Failed with %s | %s" %(type(e).__name__, str(e)) )
        #print("Open settings.xml | %s | %s" % (type(e).__name__, str(e)))
        return False

    logger_ui.info("Open settings.xml succeeded.")
    # Create SOUP to traverse XML File.
    self.soup = sop
    return self.soup


  """
  Find Environments from settings.xml
  Return: List of all environments in a File.
  """
  def find_domain(self, env_id):

    # List to be returned..
    list1 = []
    # Use SOUP to iterate through File.
    new_env_id = "deployment." + env_id.lower()
    for child in self.soup.findAll("profile"):
      if str(child.id.text).strip() == new_env_id:
        list1 = list(child.properties)
        break

    temp_list = []

    for line in list1:
      if isinstance(line, Comment):
        continue
      else:
        var = str(line).strip()
        if var != '\n' and len(var)>0:
          temp_list.append(var)
    return temp_list
  # END of FIND-DOMAIN.........................

  """
  Get environment Properties only.
  Return: list of properties
  """
  def get_env_var(self):

    env_list = []

    for child in self.soup.findAll("profile"):

      if str(child.id.text).strip().startswith("deployment."):

        env_text = str(child.id.text).strip()

        # Extract "deployment."
        env_var = env_text[11:]
        env_list.append(env_var)
    #logger_ui.info("Get env var succeeded.")
    return env_list


  """
  Get properties field <Tag, value> from domain_id.
  Return: Dict of {tag:value} for environment env.
  """
  def get_env_data(self, env_id):

    list1 = self.find_domain(env_id)
    my_dict = {}

    # Extract information <TAG,VALUE>
    for data in list1:

      if data.startswith("<"):
        my_list = re.sub(r'[<>]',' ', data).split()

        if len(my_list) > 2:
          my_tag = str(my_list[0])
          my_text = str(my_list[1])
          my_dict[my_tag] = my_text
        else:
          # Only <TAG></TAG>
          my_dict[my_list[0]] = ''
    logger_ui.info("Get env data succeeded.")
    return my_dict
  #END of GET-ENV-DATA....................

  """
  Searching TAGs in self.list
  """
  def search_tags(search_list, choice_key):

    found = False

    if choice_key in search_list:
      print(choice_key)
      found = True
    else:
      print("No such TAG exists: Try Again !")
    return found
  #END of SEARCH-TAGS


  """
  Regex Function: strip characters & symbols from
  tag-->value pair.
  Note: check line format returned to address LINE-BREAK.
  """
  def regex_tag_vals(self,text):

    tv_list = text.split("-->>")

    # TAG contains LINE-BREAK
    if tv_list[0].endswith("<br>"):
      tag_name = tv_list[0].replace("<br>","").strip()
    else:
      tag_name = tv_list[0].strip()

    # VALUE contains LINE-BREAK
    if tv_list[1].endswith("<br>"):
      tag_val = tv_list[1].replace("<br>","").strip()
    else:
      tag_val = tv_list[1].strip()
    return tag_name, tag_val
