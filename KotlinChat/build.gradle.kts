import org.jetbrains.kotlin.gradle.tasks.KotlinCompile
import com.google.protobuf.gradle.*
import org.gradle.nativeplatform.platform.internal.DefaultNativePlatform

plugins {
	idea
	id("com.google.protobuf") version "0.8.15"
	id("org.springframework.boot") version "3.1.7"
	id("io.spring.dependency-management") version "1.1.4"
	kotlin("jvm") version "1.8.22"
	kotlin("plugin.spring") version "1.8.22"
	kotlin("plugin.jpa") version "1.8.22"
//	id("io.github.lognet.grpc-spring-boot") version "5.1.5"
}

group = "com.albireo3754"
version = "0.0.1-SNAPSHOT"

java {
	sourceCompatibility = JavaVersion.VERSION_17
}

repositories {
	google()
	mavenCentral()
}

sourceSets{
	getByName("main"){
		java {
			srcDirs(
				"build/generated/source/proto/main/java",
				"build/generated/source/proto/main/kotlin"
			)
		}
	}
}

val grpcSpringBootStarterVersion = "4.4.5"
val protobufVersion = "3.15.0"
val grpcVersion = "1.36.0"
val grpcKotlinVersion = "1.0.0"

dependencies {
	implementation("org.springframework.boot:spring-boot-starter-data-jpa")
	implementation("org.springframework.boot:spring-boot-starter-data-redis")
	implementation("org.springframework.boot:spring-boot-starter-web")
	implementation("org.springframework.boot:spring-boot-starter-websocket")
	implementation("com.fasterxml.jackson.module:jackson-module-kotlin")
	implementation("org.jetbrains.kotlin:kotlin-reflect")
	developmentOnly("org.springframework.boot:spring-boot-devtools")
	runtimeOnly("com.h2database:h2")
	runtimeOnly("com.mysql:mysql-connector-j")
	testImplementation("org.springframework.boot:spring-boot-starter-test")

	// gRPC
	implementation("io.grpc:grpc-protobuf:$grpcVersion")
	implementation("io.grpc:grpc-netty-shaded:$grpcVersion")
	implementation("io.grpc:grpc-stub:$grpcVersion")
	implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.4.3")
}

tasks.withType<KotlinCompile> {
	kotlinOptions {
		freeCompilerArgs += "-Xjsr305=strict"
		jvmTarget = "17"
	}
}

tasks.withType<Test> {
	useJUnitPlatform()
}

tasks.bootBuildImage {
	builder.set("paketobuildpacks/builder-jammy-base:latest")
}

val isArm = OperatingSystem.current()

protobuf {
	protoc {
		if (project.hasProperty('protoc_platform')) {
			artifact = "com.google.protobuf:protoc:${protobufVersion}:${protoc_platform}"
		} else {
			artifact = "com.google.protobuf:protoc:${protobufVersion}"
		}
	}
	plugins {
		grpc {
			if (project.hasProperty('protoc_platform')) {
				artifact = "io.grpc:protoc-gen-grpc-java:${grpcVersion}:${protoc_platform}"
			} else {
				artifact = "io.grpc:protoc-gen-grpc-java:${grpcVersion}"
			}
		}
	}
	plugins {
		id("grpc") {
			artifact = "io.grpc:protoc-gen-grpc-java:$grpcVersion"
		}
		id("grpckt") {
			artifact = "io.grpc:protoc-gen-grpc-kotlin:$grpcKotlinVersion:jdk7@jar"
		}
	}
	generateProtoTasks {
		all().forEach { generateProtoTask ->
			generateProtoTask.plugins {
				id("grpc")
				id("grpckt")
			}
		}
	}
}