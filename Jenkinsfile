def obabeldir = '/opt/openbabel-3.1.1-static';
def mingwdir = '/usr/i686-w64-mingw32/sys-root/mingw/bin'
def openssldir = '/opt/openssl-1.1.1n-mingw32/bin'
def blogUrl= ''

pipeline {
  agent any
  // TODO determine version and check it does not yet exist
  // TODO push tag to github

  stages {
    stage('Version') {
      steps {
        dir('sources') {
          script {
            env.msk_version = readFile 'version'
            env.msk_version -= '\n'
            env.msk_version_nick = readFile 'versionnick'
            env.msk_version_nick -= '\n'
          }
          echo "Build version:  ${env.msk_version_nick} ${env.msk_version}"
        }
      }
    }
    stage('Build') {
      steps {
        dir('mainbuild') {
          sh '''
            qmake-qt5 ../sources/Molsketch.pro \
            CONFIG+=release
          '''
          sh 'make -j 8'
        }
      }
    }
    stage('Test') {
      steps {
        dir('testbuild') {
          sh '''
            qmake-qt5 ../sources/tests \
              -spec linux-g++ \
              CONFIG+=debug \
              CONFIG-=qml_debug \
              CXXTEST_INCLUDE_PATH=/opt/cxxtest-4.4 \
              CXXTEST_BIN_PATH=/opt/cxxtest-4.4
          '''
          sh 'make -j 8'
          sh 'xvfb-run ./msktests'
        }
      }
      post {
        always {
          dir('testbuild') {
            junit checksName: 'Test report collection', keepLongStdio: true, testResults: 'TEST-cxxtest.xml'
          }
        }
      }
    }
    stage('Package') {
      steps {
        dir('sources') {
          sh 'git archive HEAD -o Molsketch-${msk_version}-src.tar.gz' // TODO exclude infrastructure files!
        }
      }
      post {
        success {
          archiveArtifacts artifacts: "sources/Molsketch-${env.msk_version}-src.tar.gz", followSymlinks: false
        }
      }
    }
    stage('WinBuild') {
      steps {
        dir('winbuild') {
          sh '''
            /opt/Qt-5.15.5-mingw32-static/bin/qmake \
            CONFIG-=debug CONFIG+=release \
            DEFINES+=THIRD_PARTY_LICENSES \
            OB_LIBRARY_DIRS+=/opt/openbabel-3.1.1-static/lib/ \
            OB_INCLUDE_DIRS+=/opt/openbabel-3.1.1-static/include/openbabel3 \
            ../sources/Molsketch.pro
          '''
          sh 'make -j 8'
        }
      }
      post {
        success {
          dir('winbuild') {
            stash includes: 'bin/molsketch.exe', name: 'mainExecutable'
            stash includes: 'lib/obabeliface.dll', name: 'obabeliface'
          }
        }
      }
    }
    stage('WinInstallerSetup') {
      steps {
        dir('sources/wininstaller/packages/org.molsketch/data') {
          unstash 'mainExecutable'
        }
        dir('sources/wininstaller/packages/org.molsketch.openbabel.adapter/data') {
          unstash 'obabeliface'
        }
        dir(obabeldir) {
          fileOperations([
            folderCreateOperation("${WORKSPACE}/sources/wininstaller/packages/org.openbabel.formats/data"),
            fileCopyOperation(flattenFiles: true, includes: "**/*.obf", targetLocation: "${WORKSPACE}/sources/wininstaller/packages/org.openbabel.formats/data"),
            folderCreateOperation("${WORKSPACE}/sources/wininstaller/packages/org.openbabel.mainlib/data"),
            fileCopyOperation(flattenFiles: true, includes: "**/*.dll", targetLocation: "${WORKSPACE}/sources/wininstaller/packages/org.openbabel.mainlib/data"),
          ])
        }
        dir(mingwdir) {
          fileOperations([
            fileCopyOperation(flattenFiles: true, includes: '**/libstdc++-6.dll', targetLocation: "${WORKSPACE}/sources/wininstaller/packages/org.openbabel.mainlib/data/"),
            fileCopyOperation(flattenFiles: true, includes: '**/libgcc_s_sjlj-1.dll', targetLocation: "${WORKSPACE}/sources/wininstaller/packages/org.openbabel.mainlib/data/"),
            fileCopyOperation(flattenFiles: true, includes: '**/libwinpthread-1.dll', targetLocation: "${WORKSPACE}/sources/wininstaller/packages/org.openbabel.mainlib/data/"),
          ])
        }
        dir(openssldir) {
          fileOperations([
            folderCreateOperation("${WORKSPACE}/sources/wininstaller/packages/org.openssl.lib/data/"),
            fileCopyOperation(flattenFiles: true, includes: '**/*.dll', targetLocation: "${WORKSPACE}/sources/wininstaller/packages/org.openssl.lib/data/")
          ])
        }
      }
    }
    stage('WinRepoGen') {
      steps {
        dir("sources/wininstaller") {
          // TODO introduce variable for packages to exclude -e org.openbabel,org.openbabel.formats,org.openbabel.mainlib,org.openssl.lib
          sh "/opt/qt-installer-fw-win/bin/repogen.exe -p packages --update ${env.msk_version}"
          sh "xsltproc -o Updates-new.xml --stringparam mskversion \"${env.msk_version}\" updatexml.xsl \"${env.msk_version}\"/Updates.xml && mv Updates-new.xml \"${env.msk_version}\"/Updates.xml"
        }
      }
    }
    stage('WinRepoUpdate') {
      steps {
        dir("winrepo") {
          git branch: 'main', changelog: false, credentialsId: 'github', poll: false, url: 'git@github.com:hvennekate/molsketch-repository.git'
        }
        // fileOperations may be undesirable: does not fail if source does not exist!
        fileOperations([folderCopyOperation(destinationFolderPath: "winrepo/windows/${env.msk_version}", sourceFolderPath: "sources/wininstaller/${env.msk_version}")])
        dir("winrepo") {
          sh "xsltproc -o Updates-old.xml --stringparam mskversion \"${env.msk_version}\" ../sources/wininstaller/update_old_repo.xsl windows/\$(cd windows && ls | sort --version-sort | tail -n 1)/Updates.xml && mv Updates-old.xml windows/\$(cd windows && ls | sort --version-sort | tail -n 1)/Updates.xml"
          sh "git add . && git commit -m \"add repo for version ${env.msk_version}\""
          git push
        }
      }
    }
    stage('Sourceforge') {
        //     // select default https://sourceforge.net/p/forge/documentation/Using%20the%20Release%20API/
      steps {
        unarchive mapping: ["sources/Molsketch-${env.msk_version}-src.tar.gz": "Molsketch-${env.msk_version}-src.tar.gz"]
        withCredentials([sshUserPrivateKey(credentialsId: 'sourceforge_ssh', keyFileVariable: 'sourceForgeKey', passphraseVariable: '', usernameVariable: 'sourceForgeUser')]) {
          sshPut remote: [allowAnyHosts: true, name: 'SourceForge', host: 'frs.sourceforge.net', identityFile: sourceForgeKey, user: sourceForgeUser], from: "sources/Molsketch${env.msk_version}-src.tar.gz", into: '/home/frs/project/molsketch/Molsketch/'
        }
        withCredentials([string(credentialsId: 'sourceforge-api-token', variable: "apiKey"]) {
          httpRequest acceptType: 'APPLICATION_JSON', customHeaders: [[maskValue: false, name: 'Content-Type', value: 'application/x-www-form-urlencoded']], httpMode: 'PUT', consoleLogResponseBody: true, requestBody: "default=linux&default=mac&default=bsd&default=solaris&default=others&api_key=${apiKey}", responseHandle: 'NONE', url: "https://sourceforge.net/projects/molsketch/files/Molsketch/Molsketch-${env.msk_version}-src.tar.gz", wrapAsMultipart: true
        }
      }
    }
    stage('Homepage') {
      steps {
        // Add blog post https://anypoint.mulesoft.com/apiplatform/sourceforge/#/portals/organizations/98f11a03-7ec0-4a34-b001-c1ca0e0c45b1/apis/32951/versions/34322
        input message: 'Blog post', ok: 'Post', parameters: [
          string(description: 'Title', name: 'title'),
          text(description: 'Blogpost to be published on the Molsketch project page', name: 'blogpost')
          ]
        withCredentials([string(credentialsId: 'sourceforge_bearer', variable: 'bearerToken')]) {
          httpRequest acceptType: 'APPLICATION_JSON', customHeaders: [[maskValue: false, name: 'Content-Type', value: 'application/x-www-form-urlencoded'], [maskValue: true, name: 'Authorization', value: "Bearer ${bearerToken}"]], httpMode: 'POST', consoleLogResponseBody: true, requestBody: "labels=test1,test2&state=published&text=${java.net.URLEncoder.encode(blogpost, 'UTF-8')}&title=${title}", responseHandle: 'NONE', url: 'https://sourceforge.net/rest/p/molsketch/blog', wrapAsMultipart: false
          script {
            def response = httpRequest(acceptType: 'APPLICATION_JSON', customHeaders: [[maskValue: true, name: 'Authorization', value: "Bearer ${bearerToken}"]], httpMode: 'GET', url: 'https://sourceforge.net/rest/p/molsketch/blog')
            def responseObject = readJson text: response.content
            env.blogUrl = responseObject.posts[0].url.replaceFirst("/rest", "")
          }
        }
        echo "Blog published: ${env.blogUrl}"
        dir('homepage') {
          sh 'wget https://molsketch.sourceforge.io'
          sh 'xsltproc --stringparam mskversion "${msk_version}" --stringparam mskversionnick "${msk_version_nick}" --stringparam date "$(date --iso-8601)" --stringparam blogpostUrl "${blogpostUrl}" --html ../homepage-update.xsl index.html'
        }
        withCredentials([sshUserPrivateKey(credentialsId: 'sourceforge_ssh', keyFileVariable: 'sourceForgeKey', passphraseVariable: '', usernameVariable: 'sourceForgeUser')]) {
          sshPut remote: [allowAnyHosts: true, name: 'SourceForge', host: 'web.sourceforge.net', identityFile: sourceForgeKey, user: sourceForgeUser], from: "homepage/index.html", into: '/home/project-web/molsketch/htdocs/index.html'
        }
      }
    }
// TODO update README.md on sourceforge!
// TODO add toggle to choose update of installer
//     stage('WinInstaller') {
//       steps {
//         dir("sources/wininstaller") {
//           sh '/opt/qt-installer-fw-win/bin/binarycreator.exe -c config/config.xml -p packages -f MolsketchInstaller.exe'
//         }
//       }
//     }
  }
}
