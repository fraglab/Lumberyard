def need_bootstrap(){
	if (params.containsKey('BOOTSTRAP'))		
		return BOOTSTRAP == "true"
	else 
		return true
}

timestamps{
	node('win_git_build_slave') {
		ws("C:\\GIT_${BRANCH_NAME}"){
			def bootstrap = need_bootstrap()
			properties([disableConcurrentBuilds(),
						parameters([booleanParam(defaultValue: false, description: 'Run git_bootstrap.exe', name: 'BOOTSTRAP')])])
						
			stage('Checkout'){
				checkout scm
			}
		   
			stage('Build'){
				if (bootstrap)
					bat "git_bootstrap.exe -s --skipSetupAssistant"
				dir("dev"){
					bat """\
						Tools\\LmbrSetup\\Win\\SetupAssistantBatch.exe ^
						--3rdpartypath C:\\3rdparty ^
						--disablecapability rungame ^
						--disablecapability runeditor ^
						--enablecapability compilegame ^
						--enablecapability compileengine ^
						--enablecapability compilesandbox ^
						""".stripIndent()
					bat "lmbr_waf.bat --use-incredibuild true build_win_x64_vs2015_profile_test -p all"
				}
			}
			
			stage('Test'){
				def GOT_ERROR = false
				def ERROR_TEXT
				dir("dev"){
					bat 'rd TestResults /S /Q || Echo TestResults already absent'
					try {
						bat 'lmbr_test.cmd scan --dir Bin64vc140.Test'
					} catch (err){ 
						GOT_ERROR = true
						ERROR_TEXT = "Test error: ${err}"
					}					
					junit allowEmptyResults: true, testResults: 'TestResults\\*\\*.xml'
				}				
			}
			if (GOT_ERROR){
				error ERROR_TEXT
			}
		}
	}
}
