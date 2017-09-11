timestamps{	
	node('win_git_build_slave') {
		ws("C:\\GIT_${BRANCH_NAME}"){
			properties([parameters([booleanParam(defaultValue: false, description: 'Run git_bootstrap.exe', name: 'BOOTSTRAP')]), pipelineTriggers([])])
			properties([disableConcurrentBuilds()])
			
			stage('Checkout'){
				checkout scm
			}
		   
			stage('Build'){
				if (BOOTSTRAP)
					bat "git_bootstrap.exe -k -s --skipSetupAssistant"
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
				dir("dev"){
					bat "lmbr_test.cmd scan --dir Bin64vc140.Test"
				}
			}
		}
	}
}
