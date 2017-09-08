timestamps{
	node('win_git_build_slave') {
		properties([disableConcurrentBuilds()])
		
		stage('Checkout'){
			checkout scm
		}
	   
		stage('Build'){
			bat "git_bootstrap.exe -k -s --skipSetupAssistant"
			dir("dev"){
				bat "Tools\\LmbrSetup\\Win\\SetupAssistantBatch.exe --3rdpartypath C:\3rdparty --enablecapability compilegame --enablecapability compileengine --enablecapability compilesandbox"
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