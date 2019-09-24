import com.atlassian.bamboo.specs.api.BambooSpec;
import com.atlassian.bamboo.specs.api.builders.BambooKey;
import com.atlassian.bamboo.specs.api.builders.BambooOid;
import com.atlassian.bamboo.specs.api.builders.Variable;
import com.atlassian.bamboo.specs.api.builders.applink.ApplicationLink;
import com.atlassian.bamboo.specs.api.builders.notification.Notification;
import com.atlassian.bamboo.specs.api.builders.permission.PermissionType;
import com.atlassian.bamboo.specs.api.builders.permission.Permissions;
import com.atlassian.bamboo.specs.api.builders.permission.PlanPermissions;
import com.atlassian.bamboo.specs.api.builders.plan.Job;
import com.atlassian.bamboo.specs.api.builders.plan.Plan;
import com.atlassian.bamboo.specs.api.builders.plan.PlanIdentifier;
import com.atlassian.bamboo.specs.api.builders.plan.Stage;
import com.atlassian.bamboo.specs.api.builders.plan.artifact.Artifact;
import com.atlassian.bamboo.specs.api.builders.plan.branches.BranchCleanup;
import com.atlassian.bamboo.specs.api.builders.plan.branches.PlanBranchManagement;
import com.atlassian.bamboo.specs.api.builders.plan.configuration.ConcurrentBuilds;
import com.atlassian.bamboo.specs.api.builders.project.Project;
import com.atlassian.bamboo.specs.api.builders.repository.VcsChangeDetection;
import com.atlassian.bamboo.specs.api.builders.requirement.Requirement;
import com.atlassian.bamboo.specs.builders.notification.FirstFailedJobNotification;
import com.atlassian.bamboo.specs.builders.notification.ResponsibleRecipient;
import com.atlassian.bamboo.specs.builders.repository.bitbucket.server.BitbucketServerRepository;
import com.atlassian.bamboo.specs.builders.repository.viewer.BitbucketServerRepositoryViewer;
import com.atlassian.bamboo.specs.builders.task.ArtifactDownloaderTask;
import com.atlassian.bamboo.specs.builders.task.CheckoutItem;
import com.atlassian.bamboo.specs.builders.task.CommandTask;
import com.atlassian.bamboo.specs.builders.task.DockerBuildImageTask;
import com.atlassian.bamboo.specs.builders.task.DockerPushImageTask;
import com.atlassian.bamboo.specs.builders.task.DockerRunContainerTask;
import com.atlassian.bamboo.specs.builders.task.DownloadItem;
import com.atlassian.bamboo.specs.builders.task.InjectVariablesTask;
import com.atlassian.bamboo.specs.builders.task.ScriptTask;
import com.atlassian.bamboo.specs.builders.task.TestParserTask;
import com.atlassian.bamboo.specs.builders.task.VcsCheckoutTask;
import com.atlassian.bamboo.specs.builders.trigger.BitbucketServerTrigger;
import com.atlassian.bamboo.specs.model.task.InjectVariablesScope;
import com.atlassian.bamboo.specs.model.task.ScriptTaskProperties;
import com.atlassian.bamboo.specs.model.task.TestParserTaskProperties;
import com.atlassian.bamboo.specs.util.BambooServer;

@BambooSpec
public class PlanSpec {

    public Plan plan() {
        final Plan plan = new Plan(new Project()
                .oid(new BambooOid("yfhea8w6cete"))
                .key(new BambooKey("BHID2"))
                .name("ASAPO"),
            "ASAPO Main",
            new BambooKey("BT"))
            .oid(new BambooOid("yf7p2niyiryb"))
            .description("Build ASAPO software and run all tests")
            .pluginConfigurations(new ConcurrentBuilds()
                    .useSystemWideDefault(false)
                    .maximumNumberOfConcurrentBuilds(3))
            .stages(new Stage("Build Debug And Test")
                    .jobs(new Job("Linux - Debug",
                            new BambooKey("BUILD"))
                            .description("Builds ASAPO and tests")
                            .artifacts(new Artifact()
                                    .name("libcommon.so")
                                    .copyPattern("libcommon.so")
                                    .location("build/common/cpp/"),
                                new Artifact()
                                    .name("inotify-event-detector-cpp")
                                    .copyPattern("inotify-event-detector-cpp")
                                    .location("build/producer/inotify-event-detector-cpp/"),
                                new Artifact()
                                    .name("Documentation")
                                    .copyPattern("**/*")
                                    .location("doxygen/html"),
                                new Artifact()
                                    .name("libproducer-api.so")
                                    .copyPattern("libproducer-api.so")
                                    .location("build/producer/api/"),
                                new Artifact()
                                    .name("Coverage-Producer")
                                    .copyPattern("**/*")
                                    .location("build/coverage-asapo-producer"),
                                new Artifact()
                                    .name("Coverage-Consumer")
                                    .copyPattern("**/*")
                                    .location("build/coverage-hidra2-consumer"),
                                new Artifact()
                                    .name("Coverage-Broker")
                                    .copyPattern("coverage.html")
                                    .location("build/broker"))
                            .tasks(new VcsCheckoutTask()
                                    .description("Checkout Default Repository")
                                    .checkoutItems(new CheckoutItem().defaultRepository())
                                    .cleanCheckout(true),
                                new CommandTask()
                                    .description("get submodules")
                                    .executable("bash")
                                    .argument("-c \"git submodule init && git submodule update\""),
                                new CommandTask()
                                    .description("recreate build folder")
                                    .executable("bash")
                                    .argument("-c \"mkdir build\""),
                                new CommandTask()
                                    .description("get go modules for broker")
                                    .executable("bash")
                                    .argument("-c \"go get ./...\" || echo go modules not updated")
                                    .environmentVariables("PATH=$PATH:/usr/local/go/bin GOPATH=/opt/asapo/go:${bamboo.build.working.directory}/broker:${bamboo.build.working.directory}/common/go")
                                    .workingSubdirectory("broker"),
                                new CommandTask()
                                    .description("get go modules for discovery service")
                                    .executable("bash")
                                    .argument("-c \"go get ./...\" || echo go modules not updated")
                                    .environmentVariables("PATH=$PATH:/usr/local/go/bin GOPATH=/opt/asapo/go:${bamboo.build.working.directory}/discovery:${bamboo.build.working.directory}/common/go")
                                    .workingSubdirectory("discovery"),
                                new CommandTask()
                                    .description("build")
                                    .executable("bash")
                                    .argument("-c \"/opt/asapo/cmake-3.7.2/bin/cmake -DLIBCURL_DIR=/opt/asapo/libcurl -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_DOCS=ON -DBUILD_INTEGRATION_TESTS=ON -DBUILD_EXAMPLES=ON -DBUILD_CONSUMER_TOOLS=ON -DBUILD_BROKER=ON .. && make\"")
                                    .environmentVariables("PATH=$PATH:/usr/local/go/bin GOPATH=/opt/asapo/go GOROOT=/usr/local/go")
                                    .workingSubdirectory("build"),
                                new CommandTask()
                                    .description("doxygen")
                                    .enabled(false)
                                    .executable("Doxygen")
                                    .argument("doxygen.ini"),
                                new CommandTask()
                                    .description("Make documentation")
                                    .executable("bash")
                                    .argument("-c \"ls -la .. && make documentation && ls -la ../doxygen/html\"")
                                    .workingSubdirectory("build"),
                                new CommandTask()
                                    .description("ls after build")
                                    .executable("bash")
                                    .argument("-c \"pwd; ls -la\"")
                                    .workingSubdirectory("build"),
                                new DockerRunContainerTask()
                                    .enabled(false)
                                    .imageName("docker.io/utgarda/debian-build-essentials-git-cmake:latest")
                                    .serviceURLPattern("http://localhost:${docker.port}")
                                    .containerCommand("cmake")
                                    .containerWorkingDirectory("/data")

                                    .clearVolumeMappings()
                                    .appendVolumeMapping("${bamboo.working.directory}", "/data"),
                                new CommandTask()
                                    .description("run all tests")
                                    .executable("bash")
                                    .argument("-c \"/opt/asapo/cmake-3.7.2/bin/ctest --no-compress-output -T Test -V\"")
                                    .environmentVariables("PATH=$PATH:/usr/local/go/bin GOPATH=/opt/asapo/go")
                                    .workingSubdirectory("build"))
                            .finalTasks(new CommandTask()
                                    .description("convert tests")
                                    .executable("bash")
                                    .argument("-c \"python3 ../3d_party/ctest_junit_convert/convert.py -x ../3d_party/ctest_junit_convert/conv.xsl -t . > Testing/JUnitTestResults.xml\"")
                                    .workingSubdirectory("build"),
                                new TestParserTask(TestParserTaskProperties.TestType.JUNIT)
                                    .resultDirectories("**/Testing/*.xml"))
                            .requirements(new Requirement("system.docker.executable")),
                        new Job("Windows - Debug",
                            new BambooKey("BOW"))
                            .description("Builds ASAPO and tests")
                            .tasks(new VcsCheckoutTask()
                                    .description("Checkout Default Repository")
                                    .checkoutItems(new CheckoutItem().defaultRepository())
                                    .cleanCheckout(true),
                                new ScriptTask()
                                    .description("Create build folder")
                                    .interpreter(ScriptTaskProperties.Interpreter.BINSH_OR_CMDEXE)
                                    .inlineBody("mkdir build"),
                                new ScriptTask()
                                    .description("Go modules")
                                    .interpreter(ScriptTaskProperties.Interpreter.BINSH_OR_CMDEXE)
                                    .inlineBody("cd discovery\nSET GOPATH=\"c:\\GoPath\";\"${bamboo.build.working.directory}\\discovery\";\"${bamboo.build.working.directory}\\common\\go\"\ngo get ./... || echo go modules not updated\ncd ../broker\nSET GOPATH=\"c:\\GoPath\";\"${bamboo.build.working.directory}\\broker\";\"${bamboo.build.working.directory}\\common\\go\"\ngo get ./... || echo go modules not updated"),
                                new ScriptTask()
                                    .description("build with CMake")
                                    .interpreter(ScriptTaskProperties.Interpreter.BINSH_OR_CMDEXE)
                                    .inlineBody("SET GOPATH=\"c:\\GoPath\"\n\"c:\\Program Files\\CMake\\bin\\cmake\" -DLIBCURL_DIR=c:/Curl -Dgtest_SOURCE_DIR=c:/googletest -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_DOCS=ON -DBUILD_INTEGRATION_TESTS=ON -DBUILD_EXAMPLES=ON -DBUILD_CONSUMER_TOOLS=ON -DBUILD_BROKER=ON -Dlibmongoc-static-1.0_DIR=\"c:\\mongo-c-driver\\lib\\cmake\\libmongoc-static-1.0\" -Dlibbson-static-1.0_DIR=\"c:\\mongo-c-driver\\lib\\cmake\\libbson-static-1.0\" ..\n\"c:\\Program Files\\CMake\\bin\\cmake\" --build .")
                                    .workingSubdirectory("build"),
                                new ScriptTask()
                                    .description("Run tests")
                                    .interpreter(ScriptTaskProperties.Interpreter.BINSH_OR_CMDEXE)
                                    .inlineBody("\"c:\\Program Files\\CMake\\bin\\ctest\" -C Debug -T test --no-compress-output -V")
                                    .workingSubdirectory("build"))
                            .finalTasks(new ScriptTask()
                                    .inlineBody("\"c:\\Program Files (x86)\\Python\\python.exe\" ..\\3d_party\\ctest_junit_convert\\convert.py -x ..\\3d_party\\ctest_junit_convert/conv.xsl -t . > Testing\\JUnitTestResults.xml")
                                    .workingSubdirectory("build"),
                                new TestParserTask(TestParserTaskProperties.TestType.JUNIT)
                                    .resultDirectories("**/Testing/*.xml"))
                            .requirements(new Requirement("system.builder.devenv.Visual Studio 2015 CE"))),
                new Stage("Build Release")
                    .jobs(new Job("Windows - Release",
                            new BambooKey("WR"))
                            .artifacts(new Artifact()
                                    .name("Dummy Producer Windows")
                                    .copyPattern("dummy-data-producer.exe")
                                    .location("build/examples/producer/dummy-data-producer")
                                    .shared(true),
                                new Artifact()
                                    .name("File Monitor Producer Windows")
                                    .copyPattern("asapo-eventmon-producer.exe")
                                    .location("build/producer/event_monitor_producer")
                                    .shared(true))
                            .tasks(new VcsCheckoutTask()
                                    .description("Checkout Default Repository")
                                    .checkoutItems(new CheckoutItem().defaultRepository())
                                    .cleanCheckout(true),
                                new ScriptTask()
                                    .description("Create build folder")
                                    .interpreter(ScriptTaskProperties.Interpreter.BINSH_OR_CMDEXE)
                                    .inlineBody("mkdir build"),
                                new ScriptTask()
                                    .description("build with CMake")
                                    .interpreter(ScriptTaskProperties.Interpreter.BINSH_OR_CMDEXE)
                                    .inlineBody("SET GOPATH=\"c:\\GoPath\"\n\"c:\\Program Files\\CMake\\bin\\cmake\" -DLIBCURL_DIR=c:/Curl -Dgtest_SOURCE_DIR=c:/googletest -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_DOCS=OFF -DBUILD_INTEGRATION_TESTS=OFF -DBUILD_EXAMPLES=ON -DBUILD_CONSUMER_TOOLS=ON -DBUILD_BROKER=ON -Dlibmongoc-static-1.0_DIR=\"c:\\mongo-c-driver\\lib\\cmake\\libmongoc-static-1.0\" -Dlibbson-static-1.0_DIR=\"c:\\mongo-c-driver\\lib\\cmake\\libbson-static-1.0\" ..\n\"c:\\Program Files\\CMake\\bin\\cmake\" --build .")
                                    .workingSubdirectory("build"),
                                new ScriptTask()
                                    .description("Run tests")
                                    .interpreter(ScriptTaskProperties.Interpreter.BINSH_OR_CMDEXE)
                                    .inlineBody("\"c:\\Program Files\\CMake\\bin\\ctest\" -C Debug -T test --no-compress-output -V")
                                    .workingSubdirectory("build"),
                                new ScriptTask()
                                    .description("Push binary")
                                    .enabled(false)
                                    .interpreter(ScriptTaskProperties.Interpreter.WINDOWS_POWER_SHELL)
                                    .inlineBody("git checkout -B ${bamboo.planRepository.branchName}\ngit push -u origin ${bamboo.planRepository.branchName}\ncopy ../build/examples/producer/dummy-data-producer/dummy-producer.exe .\ngit add -A .\ngit commit -m \"update from ${bamboo.buildNumber}\"\ngit push")
                                    .workingSubdirectory("bin"))
                            .finalTasks(new ScriptTask()
                                    .inlineBody("\"c:\\Program Files (x86)\\Python\\python.exe\" ..\\3d_party\\ctest_junit_convert\\convert.py -x ..\\3d_party\\ctest_junit_convert/conv.xsl -t . > Testing\\JUnitTestResults.xml")
                                    .workingSubdirectory("build"),
                                new TestParserTask(TestParserTaskProperties.TestType.JUNIT)
                                    .resultDirectories("**/Testing/*.xml"))
                            .requirements(new Requirement("system.builder.devenv.Visual Studio 2015 CE")),
                        new Job("Linux - Release",
                            new BambooKey("LIN"))
                            .artifacts(new Artifact()
                                    .name("Receiver")
                                    .copyPattern("receiver")
                                    .location("build_release/receiver")
                                    .shared(true),
                                new Artifact()
                                    .name("Authorizer")
                                    .copyPattern("asapo-authorizer")
                                    .location("build_release/authorizer")
                                    .shared(true),
                                new Artifact()
                                    .name("Discovery")
                                    .copyPattern("asapo-discovery")
                                    .location("build_release/discovery")
                                    .shared(true),
                                new Artifact()
                                    .name("Broker")
                                    .copyPattern("asapo-broker")
                                    .location("build_release/broker")
                                    .shared(true),
                                new Artifact()
                                    .name("Dummy Producer Linux")
                                    .copyPattern("dummy-data-producer")
                                    .location("build_release/examples/producer/dummy-data-producer")
                                    .shared(true),
                                new Artifact()
                                    .name("File Monitor Producer Linux")
                                    .copyPattern("asapo-eventmon-producer")
                                    .location("build_release/producer/event_monitor_producer")
                                    .shared(true),
                                new Artifact()
                                    .name("Consumer Linux")
                                    .copyPattern("getnext_broker")
                                    .location("build_release/examples/consumer/getnext_broker")
                                    .shared(true))
                            .tasks(new VcsCheckoutTask()
                                    .checkoutItems(new CheckoutItem().defaultRepository())
                                    .cleanCheckout(true),
                                new CommandTask()
                                    .description("recreate build folder")
                                    .executable("bash")
                                    .argument("-c \"mkdir build_release\""),
                                new CommandTask()
                                    .description("build")
                                    .executable("bash")
                                    .argument("-c \"/opt/asapo/cmake-3.7.2/bin/cmake -DLIBCURL_DIR=/opt/asapo/libcurl -DCMAKE_BUILD_TYPE=Release  -DBUILD_EXAMPLES=ON -DBUILD_CONSUMER_TOOLS=ON -DBUILD_BROKER=ON .. && make\"")
                                    .environmentVariables("PATH=$PATH:/usr/local/go/bin GOPATH=/opt/asapo/go GOROOT=/usr/local/go")
                                    .workingSubdirectory("build_release"),
                                new CommandTask()
                                    .description("run all tests")
                                    .executable("bash")
                                    .argument("-c \"/opt/asapo/cmake-3.7.2/bin/ctest --no-compress-output -T Test -V\"")
                                    .environmentVariables("PATH=$PATH:/usr/local/go/bin GOPATH=/opt/asapo/go")
                                    .workingSubdirectory("build"))
                            .finalTasks(new CommandTask()
                                    .description("convert tests")
                                    .executable("bash")
                                    .argument("-c \"python3 ../3d_party/ctest_junit_convert/convert.py -x ../3d_party/ctest_junit_convert/conv.xsl -t . > Testing/JUnitTestResults.xml\"")
                                    .workingSubdirectory("build"),
                                new TestParserTask(TestParserTaskProperties.TestType.JUNIT)
                                    .resultDirectories("**/Testing/*.xml"))),
                new Stage("Create Docker Contaners")
                    .jobs(new Job("Create Docker",
                            new BambooKey("DOC"))
                            .tasks(new VcsCheckoutTask()
                                    .checkoutItems(new CheckoutItem().defaultRepository()),
                                new ScriptTask()
                                    .description("prepare version file")
                                    .inlineBody("set -e\ntag=`git describe --tags --dirty`\nif [ \"${bamboo.planRepository.branchName}\" = \"master\" ]; then\necho asapo_tag=$tag > version\necho asapo_dev=\"\" >> version\nelif [ \"${bamboo.planRepository.branchName}\" = \"develop\" ]; then\necho asapo_tag=${bamboo.planRepository.branchName}.${tag} > version\necho asapo_dev=\"-dev\" >> version\nelse\necho asapo_tag=${bamboo.planRepository.branchName}.latest > version\necho asapo_dev=\"-dev\" >> version\nfi"),
                                new InjectVariablesTask()
                                    .description("Set docker tag")
                                    .path("version")
                                    .namespace("inject")
                                    .scope(InjectVariablesScope.RESULT),
                                new ArtifactDownloaderTask()
                                    .description("Copy All")
                                    .artifacts(new DownloadItem()
                                            .artifact("Broker")
                                            .path("broker/docker"),
                                        new DownloadItem()
                                            .artifact("Authorizer")
                                            .path("authorizer/docker"),
                                        new DownloadItem()
                                            .artifact("Discovery")
                                            .path("discovery/docker"),
                                        new DownloadItem()
                                            .artifact("Receiver")
                                            .path("receiver/docker")),
                                new DockerBuildImageTask()
                                    .description("Build image")
                                    .enabled(false)
                                    .workingSubdirectory("receiver/docker")
                                    .imageName("yakser/asapo-receiver${bamboo.inject.asapo_dev}:${bamboo.inject.asapo_tag}")
                                    .useCache(true)
                                    .dockerfileInWorkingDir(),
                                new DockerPushImageTask()
                                    .dockerHubImage("yakser/asapo-receiver${bamboo.inject.asapo_dev}:${bamboo.inject.asapo_tag}")
                                    .authentication("yakubov", /*FIXME put your password here*/),
                                new ScriptTask()
                                    .description("Build Dockers")
                                    .inlineBody("set -e\n\nservices=\"broker authorizer discovery receiver\"\n\ndocker login -u=${bamboo.docker_username} -p=${bamboo.docker_userpassword}\n\n\nfor service in $services\ndo\n cd ${service}/docker\n docker build -t yakser/asapo-${service}${bamboo.inject.asapo_dev}:${bamboo.inject.asapo_tag} .\n docker push yakser/asapo-${service}${bamboo.inject.asapo_dev}:${bamboo.inject.asapo_tag}\n cd -\ndone\n\ndocker logout"))
                            .requirements(new Requirement("system.docker.executable"))),
                new Stage("Push Binaries")
                    .jobs(new Job("Push Binaries",
                            new BambooKey("PWB"))
                            .tasks(new ArtifactDownloaderTask()
                                    .description("Get Binaries")
                                    .artifacts(new DownloadItem()
                                            .artifact("Dummy Producer Linux"),
                                        new DownloadItem()
                                            .artifact("Consumer Linux"),
                                        new DownloadItem()
                                            .artifact("File Monitor Producer Windows"),
                                        new DownloadItem()
                                            .artifact("File Monitor Producer Linux"),
                                        new DownloadItem()
                                            .artifact("Dummy Producer Windows")),
                                new ScriptTask()
                                    .description("Push binaries")
                                    .inlineBody("#!/bin/bash\nset -e\n\nfor path in *; do\n if [ -f \"${path}\" ]; then\n  root=\"${path#.}\";root=\"${path%\"$root\"}${root%.*}\"\n  ext=\"${path#\"$root\"}\"\n  echo $path\n  mv \"${path}\" \"${root}-${bamboo.inject.asapo_tag}${ext}\"\n fi\ndone\n\nrsync -avr . mirror@it-fs5:/\nwget -q -O- http://it-fs5.desy.de/cgi-bin/asapoSync.cgi\n\nrm *"))
                            .requirements(new Requirement("system.docker.executable"))))
            .planRepositories(new BitbucketServerRepository()
                    .name("ASAPO")
                    .oid(new BambooOid("yfbuqh970z6t"))
                    .repositoryViewer(new BitbucketServerRepositoryViewer())
                    .server(new ApplicationLink()
                            .name("DESY Stash")
                            .id("6a33db2c-8d71-3528-b029-8c5fcbe62101"))
                    .projectKey("ASAPO")
                    .repositorySlug("asapo")
                    .sshPublicKey("ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCZHqvrcOMwtyVynQZMymalz2kKb5iaU9fhXYZp8EdM+OBFJeSTOeSksRhpNYrXSKwGH8H6yuRuEu2DF81ETJB32FVftV27KIg4i0bxd2jYPBGOf2aji7FdJkIc5e6VtSfUHKa7FiZLeXrkUjihIhQhXc+UUzvzHEpZBbYNoYf/2MztJU7djIQ/L0dXjfK0hemBzhbPmzPAQf8308PzbQIY7Fiba+LKpgJHWkQEluptBj8xJSt4l7akA6yXS3KIk3CsoCobCOKbtjK32U8OZCOdU+6Ne4ZgwHnjxH/BlZB4DpZzxx/PA+zNyzO9gl4h6OSoHFpszYxFmP7dkdVhNjLp https://bamboo.desy.de")
                    .sshPrivateKey("-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEAmR6r63DjMLclcp0GTMpmpc9pCm+YmlPX4V2GafBHTPjgRSXk\nkznkpLEYaTWK10isBh/B+srkbhLtgxfNREyQd9hVX7VduyiIOItG8Xdo2DwRjn9m\no4uxXSZCHOXulbUn1BymuxYmS3l65FI4oSIUIV3PlFM78xxKWQW2DaGH/9jM7SVO\n3YyEPy9HV43ytIXpgc4Wz5szwEH/N9PD820CGOxYm2viyqYCR1pEBJbqbQY/MSUr\neJe2pAOsl0tyiJNwrKAqGwjim7Yyt9lPDmQjnVPujXuGYMB548R/wZWQeA6Wc8cf\nzwPszcszvYJeIejkqBxabM2MRZj+3ZHVYTYy6QIDAQABAoIBAAb8XT+aoOLiGDVI\ncLckLNa3xcUSHlY8KQdBKxa+PaKHYEQHXIxTAdl5svFp3fHAgQiufj0g8JcNCgUH\nGvhYveaZ2htJvQMC8GQUBqKRnhjvdKWZrmcbwnXLfwyueNofr7d/ryOt+QVMf4zX\nK3o+gcib7RKJUZzf14yENDAfBINpoKJ2yQe42F80KagKGIxKsUqdw6EngICH61o+\n0L3aqKytCw3xwPqVqREKEAjrwkFUgZVU/LhRAfG7hV1AYaKBOV/xJw5SIzjcz8rK\nGVzfrKeI9RnjC852z9E1rkFTY6OC3Lgb+EGqWw7wEydt7Y0NsZCrqa+Qc1anU9DE\nKiQKNAECgYEA1R1PJMHB5RbueiCqINPg4jfq94CLj0BkOlSN84/hBt3WEf736fw9\nU0k7IiKOiotfd2jwUMKboB0pyaCjlq9zRqX5Aw3ssOLFDcJx3BdUY78jO6H9zdOh\nD1aTp/qXYrJpE3lCuGCFHWznuIFLC9mHlQAGlZ6tG/GaM531pcHftSkCgYEAt+61\ngF5p3U/vOAydj+r74azxfDZOWEvGJfcfM3pfahQRrO01OE5wlyF4mPrhO1CMVOVE\nmo5zwrpwe0x3qkjewwMu28B2/Tz2kWLViq8NkFTkGe67ZIP+g7i6ZHLGRdbmDGYq\npO/mARkLB4VpwKWZFOQVkShbcDvLNhe2pnGWh8ECgYEAq0/2MOv4O3nllhLv75ei\nrPaaUP7qMOtoJmOWAHZmQ6jLzoeRLmxvt6WkfVoeD0zeHxUiKSlnBJys3MHe/uBm\n4CHHPCdTXxXLpbXq5Stz0QLzBZrAdVZrn/LOmzebveEBCoBtm90q5G6JDw4QD6R7\nktEEef2l1lKuzFNsBiUE+ZECgYBjqFX1MLKhc+8EamlwkfxZwW+yQmZelufMqEHi\njXpnKmqNTJUaZf3BFSSXls80aScv1G1AZNC6AHRzifIIdKUl+nKIJJcUDNT33xoe\no0xxGF3i4yPriUz8p7luNXXSX2aT70NJzKXNkHkWYSX0eIUh+Zbp6HjqraskKuMO\n+dp6wQKBgQCddiXABL0nbUigPEfrQRAIwHMD0RpoRlHeAVjZ0XYKxuSt0glxbAP8\nDFUM845WcRcAQ4OPIGDAvN2B/u9kp+0djOWdfQOHUIKQUisgEvg7mbMhsiD2uW1F\nsSEfQkBNz2aES82dTR0aGzNE6nWihI8RofFEMa03+S6Czc3CU0QJjg==\n-----END RSA PRIVATE KEY-----\n")
                    .sshCloneUrl("ssh://git@stash.desy.de:7999/asapo/asapo.git")
                    .branch("develop")
                    .remoteAgentCacheEnabled(false)
                    .changeDetection(new VcsChangeDetection()))

            .triggers(new BitbucketServerTrigger())
            .variables(new Variable("docker_username",
                    "yakser"),
                new Variable("docker_userpassword",
                    "BAMSCRT@0@0@SXl1He3DrKr2Dso1VPIn2g=="))
            .planBranchManagement(new PlanBranchManagement()
                    .createForVcsBranch()
                    .delete(new BranchCleanup()
                        .whenRemovedFromRepositoryAfterDays(7))
                    .notificationForCommitters())
            .notifications(new Notification()
                    .type(new FirstFailedJobNotification())
                    .recipients(new ResponsibleRecipient()));
        return plan;
    }

    public PlanPermissions planPermission() {
        final PlanPermissions planPermission = new PlanPermissions(new PlanIdentifier("BHID2", "BT"))
            .permissions(new Permissions()
                    .userPermissions("tcallsen", PermissionType.VIEW, PermissionType.EDIT)
                    .userPermissions("yakubov", PermissionType.EDIT, PermissionType.VIEW, PermissionType.ADMIN, PermissionType.CLONE, PermissionType.BUILD)
                    .userPermissions("cpatzke", PermissionType.VIEW, PermissionType.EDIT, PermissionType.BUILD, PermissionType.CLONE, PermissionType.ADMIN)
                    .anonymousUserPermissionView());
        return planPermission;
    }

    public static void main(String... argv) {
        //By default credentials are read from the '.credentials' file.
        BambooServer bambooServer = new BambooServer("https://bamboo.desy.de");
        final PlanSpec planSpec = new PlanSpec();

        final Plan plan = planSpec.plan();
        bambooServer.publish(plan);

        final PlanPermissions planPermission = planSpec.planPermission();
        bambooServer.publish(planPermission);
    }
}
