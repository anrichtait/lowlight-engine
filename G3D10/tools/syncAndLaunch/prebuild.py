import getopt, platform, sys, os, socket, subprocess

# Settings
localDataFilesDir=""
remoteDataFilesDir=""
remotePlatform="windows"
remoteAddr = os.environ["G3D10_REMOTE_MACHINE"]

"""
TODO: determine remote platform automatically with the script instead of passing it as an argument
It should match the local platform because we're syncing the binaries (ie a Windows compiled binary would not work on a Ubuntu server).
"""
    
s = platform.system()
windows = (s == 'Windows') or (s == 'Microsoft')
freebsd = (s == 'FreeBSD')
linux   = (s == 'Linux')
osx     = (s == 'Darwin')
cygwin  = s.lower().startswith('cygwin')
unix    = freebsd or linux or osx or cygwin


try:
    opts, args = getopt.getopt(sys.argv[1:], "r:l:d:h", ["remotePlatform=", "localDataFilesDir=", "remoteDataFilesDir=", "--help"])
    
except Exception as e:
    print("Error getting options: ")
    print(e)
    exit(1)

for opt, arg in opts:
    if opt in ['-r', '--remotePlatform']:
        remotePlatform = arg
    if opt in ['-l', '--localDataFilesDir']:
        localDataFilesDir = arg
    if opt in ['-d', '--remoteDataFilesDir']:
        remoteDataFilesDir = arg
    if opt in ['-h', '--help']:
        print(
"""
InstantLaunch Prebuild Script: 
-r  --remotePlatform        the remote platform
-l  --localDataFilesDir     the local asset directory
-d  --remoteDataFilesDir    where the assets will be copied to remotely
-h  --help                  these instructions
"""
        )


def sanitizePath(path):
    splitString = path.replace('\\', '/').split(':')
    return '/mnt/' + splitString[0].lower() + splitString[1]    


print("Starting prebuild script...")

# TODO: start matchmakerd in a second thread on local machine
matchMakerPath = r"{g3dRoot}\G3D10\tools\syncAndLaunch\matchmaker.exe".format(g3dRoot=os.environ['g3d'])
if not os.path.exists(matchMakerPath):
    print("Matchmaker program not found...\nLooked for {}\nAborting...".format(matchMakerPath))
    exit(1)
matchMakerPid = subprocess.Popen([matchMakerPath]).pid
print("Spawning matchmaker on separate process PID: {}".format(matchMakerPid))

if windows:
    localHostname = socket.gethostname()
    if localHostname in remoteAddr or "localhost" in remoteAddr:
        print("Remote Machine is the same as local... No further action will be taken...")

    elif remotePlatform == "windows":
        # We're running matchmaker local to the client
        if localHostname not in os.popen("wsl ssh %G3D10_REMOTE_MACHINE% echo ^%G3D10_SERVICE_DISCOVERY^%").read() :
            print("Remote Machine is not pointing to our matchmaker... Aborting...")
            exit(1)

        if os.path.exists(localDataFilesDir):
            localDataFilesDir = sanitizePath(localDataFilesDir)
            if remoteDataFilesDir:
                remoteDataFilesDir = sanitizePath(remoteDataFilesDir)
            else:
                remoteDataFilesDir = localDataFilesDir

            wslRSyncCommand = "wsl rsync -ravpE --rsync-path='wsl rsync' -e ssh {}/* {}:{}".format(localDataFilesDir, remoteAddr, remoteDataFilesDir)
            print(wslRSyncCommand)
            os.system(wslRSyncCommand)
        else:
            print("Data files directory not found on local machine... skipping data files sync...")

print ("prebuild step done")

