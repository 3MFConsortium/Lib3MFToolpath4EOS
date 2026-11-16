{
	"targets": [
		{
			"target_name": "lib3mftoolpath4eos_nodeaddon",
			"sources": [ "lib3mftoolpath4eos_nodeaddon.cc", "lib3mftoolpath4eos_nodewrapper.cc", "lib3mftoolpath4eos_dynamic.cc" ],
			"cflags": [ "-fexceptions " ],
			"cflags_cc": [ "-fexceptions " ],
			"msvs_settings": {
				"VCCLCompilerTool": { "ExceptionHandling": 1 }
			},
			"conditions": [
				["OS=='win'", {	"defines": [ "_HAS_EXCEPTIONS=1" ] }]
			]
		}
	]
}

