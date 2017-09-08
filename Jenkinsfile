node('win_git_build_slave') {
	stage('Checkout'){
		checkout scm
	}
   
	stage('Build'){
		dir("dev"){
			bat "Tools\LmbrSetup\Win\SetupAssistantBatch.exe --enablecapability compilegame --enablecapability compileengine --enablecapability compilesandbox"
			bat "lmbr_waf.bat --use-incredibuild true build_win_x64_vs2015_profile_test -p all"
		}
	}
	
	stage('Test'){
		dir("dev"){
			bat "lmbr_test.cmd scan --dir Bin64vc140.Test"
		}
	}
}
