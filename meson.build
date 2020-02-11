project('gst-cencdec', 'c',
  version : '0.0.2',
  meson_version : '>= 0.48.0',
  default_options : [ 'warning_level=1',
                      'buildtype=debugoptimized' ])

gst_version = meson.project_version()
version_arr = gst_version.split('.')
gst_version_major = version_arr[0].to_int()
gst_version_minor = version_arr[1].to_int()
gst_version_micro = version_arr[2].to_int()
if version_arr.length() == 4
  gst_version_nano = version_arr[3].to_int()
else
  gst_version_nano = 0
endif

have_cxx = add_languages('cpp', required : false)

glib_req = '>= 2.44.0'
gst_req = '>= @0@.@1@.0'.format(gst_version_major, gst_version_minor)

api_version = '1.0'
soversion = 0
curversion = gst_version_minor * 100 + gst_version_micro
libversion = '@0@.@1@.0'.format(soversion, curversion)
osxversion = curversion + 1

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))
plugins = []

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
have_objc = add_languages('objc', required : false)

cdata = configuration_data()

if cc.get_id() == 'msvc'
  # Ignore several spurious warnings for things gstreamer does very commonly
  # If a warning is completely useless and spammy, use '/wdXXXX' to suppress it
  # If a warning is harmless but hard to fix, use '/woXXXX' so it's shown once
  # NOTE: Only add warnings here if you are sure they're spurious
  msvc_args = [
      '/wd4018', # implicit signed/unsigned conversion
      '/wd4146', # unary minus on unsigned (beware INT_MIN)
      '/wd4244', # lossy type conversion (e.g. double -> int)
      '/wd4305', # truncating type conversion (e.g. double -> float)
      cc.get_supported_arguments(['/utf-8']), # set the input encoding to utf-8
  ]
  add_project_arguments(msvc_args, language : 'c')
  add_project_arguments(msvc_args, language : 'cpp')
  # Disable SAFESEH with MSVC for plugins and libs that use external deps that
  # are built with MinGW
  noseh_link_args = ['/SAFESEH:NO']
else
  if cxx.has_argument('-Wno-non-virtual-dtor')
    add_project_arguments('-Wno-non-virtual-dtor', language: 'cpp')
  endif

  noseh_link_args = []
endif

if cc.has_link_argument('-Wl,-Bsymbolic-functions')
  add_project_link_arguments('-Wl,-Bsymbolic-functions', language : 'c')
endif

# Symbol visibility
if cc.get_id() == 'msvc'
  export_define = '__declspec(dllexport) extern'
elif cc.has_argument('-fvisibility=hidden')
  add_project_arguments('-fvisibility=hidden', language: 'c')
  add_project_arguments('-fvisibility=hidden', language: 'cpp')
  if have_objc
    add_project_arguments('-fvisibility=hidden', language: 'objc')
  endif
  export_define = 'extern __attribute__ ((visibility ("default")))'
else
  export_define = 'extern'
endif

# Passing this through the command line would be too messy
cdata.set('GST_API_EXPORT', export_define)

# Disable strict aliasing
if cc.has_argument('-fno-strict-aliasing')
  add_project_arguments('-fno-strict-aliasing', language: 'c')
endif
if cxx.has_argument('-fno-strict-aliasing')
  add_project_arguments('-fno-strict-aliasing', language: 'cpp')
endif

check_headers = [
  ['HAVE_DLFCN_H', 'dlfcn.h'],
  ['HAVE_FCNTL_H', 'fcntl.h'],
  ['HAVE_INTTYPES_H', 'inttypes.h'],
  ['HAVE_MEMORY_H', 'memory.h'],
#  ['HAVE_PROCESS_H', 'process.h'],
  ['HAVE_STDINT_H', 'stdint.h'],
  ['HAVE_STDLIB_H', 'stdlib.h'],
  ['HAVE_STRINGS_H', 'strings.h'],
  ['HAVE_STRING_H', 'string.h'],
  ['HAVE_SYS_IOCTL_H', 'sys/ioctl.h'],
  ['HAVE_SYS_PARAM_H', 'sys/param.h'],
  ['HAVE_SYS_SOCKET_H', 'sys/socket.h'],
  ['HAVE_SYS_STAT_H', 'sys/stat.h'],
  ['HAVE_SYS_TIME_H', 'sys/time.h'],
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['HAVE_UNISTD_H', 'unistd.h'],
]

foreach h : check_headers
  if cc.has_header(h.get(1))
    cdata.set(h.get(0), 1)
  endif
endforeach

check_functions = [
['HAVE_ASINH', 'asinh', '#include<math.h>'],
['HAVE_CLOCK_GETTIME', 'clock_gettime', '#include<time.h>'],
['HAVE_COSH', 'cosh', '#include<math.h>'],
# check token HAVE_CPU_ALPHA
# check token HAVE_CPU_ARM
# check token HAVE_CPU_CRIS
# check token HAVE_CPU_CRISV32
# check token HAVE_CPU_HPPA
# check token HAVE_CPU_I386
# check token HAVE_CPU_IA64
# check token HAVE_CPU_M68K
# check token HAVE_CPU_MIPS
# check token HAVE_CPU_PPC
# check token HAVE_CPU_PPC64
# check token HAVE_CPU_S390
# check token HAVE_CPU_SPARC
# check token HAVE_CPU_X86_64
# ['HAVE_DCGETTEXT', 'dcgettext', '#include<libintl.h>'],
# check token HAVE_DIRECTSOUND
# check token HAVE_EXPERIMENTAL
# check token HAVE_EXTERNAL
# check token HAVE_FPCLASS
# check token HAVE_GCC_ASM
['HAVE_GETPAGESIZE', 'getpagesize', '#include<unistd.h>'],
# check token HAVE_GETTEXT
# check token HAVE_GST_V4L2
# check token HAVE_IOS
['HAVE_ISINF', 'isinf', '#include<math.h>'],
# check token HAVE_LIBV4L2
['HAVE_MMAP', 'mmap', '#include<sys/mman.h>'],
# ['HAVE_MMAP64', 'mmap64', '#include<sys/mman.h>'],
# check token HAVE_OSX_AUDIO
# check token HAVE_OSX_VIDEO
# check token HAVE_RDTSC
['HAVE_SINH', 'sinh', '#include<math.h>'],
# check token HAVE_WAVEFORM
]

libm = cc.find_library('m', required : false)

foreach f : check_functions
  if cc.has_function(f.get(1), prefix : f.get(2), dependencies : libm)
    cdata.set(f.get(0), 1)
  endif
endforeach

#cdata.set('SIZEOF_CHAR', cc.sizeof('char'))
#cdata.set('SIZEOF_INT', cc.sizeof('int'))
#cdata.set('SIZEOF_LONG', cc.sizeof('long'))
#cdata.set('SIZEOF_SHORT', cc.sizeof('short'))
#cdata.set('SIZEOF_VOIDP', cc.sizeof('void*'))
#cdata.set('SIZEOF_OFF_T', cc.sizeof('off_t'))

# Here be fixmes.
# FIXME: check if this is correct
#cdata.set('HAVE_CPU_X86_64', host_machine.cpu() == 'amd64')
#cdata.set('HAVE_GCC_ASM', cc.get_id() != 'msvc')
cdata.set('VERSION', '"@0@"'.format(gst_version))
cdata.set('PACKAGE_VERSION', '"@0@"'.format(gst_version))
cdata.set('GST_LICENSE', '"LGPL"')
cdata.set('PACKAGE', '"gst-cencdec"')
cdata.set('PACKAGE_NAME', '"DASH CENC plugins"')
cdata.set('GETTEXT_PACKAGE', '"gst-cencdec"')

warning_flags = [
  '-Wmissing-declarations',
  '-Wredundant-decls',
  '-Wwrite-strings',
  '-Winit-self',
  '-Wmissing-include-dirs',
  '-Wno-multichar',
  '-Wvla',
  '-Wpointer-arith',
]

warning_c_flags = [
  '-Wmissing-prototypes',
  '-Wdeclaration-after-statement',
  '-Wold-style-definition',
  '-Waggregate-return',
]

if have_cxx
  cxx = meson.get_compiler('cpp')
endif

foreach extra_arg : warning_flags
  if cc.has_argument (extra_arg)
    add_project_arguments([extra_arg], language: 'c')
  endif
  if have_cxx and cxx.has_argument (extra_arg)
    add_project_arguments([extra_arg], language: 'cpp')
  endif
endforeach

foreach extra_arg : warning_c_flags
  if cc.has_argument (extra_arg)
    add_project_arguments([extra_arg], language: 'c')
  endif
endforeach

# Mandatory GST deps
gst_dep = dependency('gstreamer-1.0', version : gst_req,
  fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : gst_req,
  fallback : ['gstreamer', 'gst_base_dep'])
gstnet_dep = dependency('gstreamer-net-1.0', version : gst_req,
  fallback : ['gstreamer', 'gst_net_dep'])
if host_machine.system() != 'windows'
  gstcheck_dep = dependency('gstreamer-check-1.0', version : gst_req,
    fallback : ['gstreamer', 'gst_check_dep'])
endif

# gstpbutils_dep = dependency('gstreamer-pbutils-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'pbutils_dep'])
# gstallocators_dep = dependency('gstreamer-allocators-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'allocators_dep'])
# gstapp_dep = dependency('gstreamer-app-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'app_dep'])
# gstaudio_dep = dependency('gstreamer-audio-1.0', version : gst_req,
#      fallback : ['gst-plugins-base', 'audio_dep'])
# gstfft_dep = dependency('gstreamer-fft-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'fft_dep'])
# gstriff_dep = dependency('gstreamer-riff-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'riff_dep'])
# gstrtp_dep = dependency('gstreamer-rtp-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'rtp_dep'])
# gstrtsp_dep = dependency('gstreamer-rtsp-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'rtsp_dep'])
# gstsdp_dep = dependency('gstreamer-sdp-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'sdp_dep'])
# gsttag_dep = dependency('gstreamer-tag-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'tag_dep'])
# gstvideo_dep = dependency('gstreamer-video-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'video_dep'])

# # GStreamer OpenGL
# gstgl_dep = dependency('gstreamer-gl-1.0', version : gst_req,
#     fallback : ['gst-plugins-base', 'gstgl_dep'], required: false)

# required glib libraries

glib_deps = [dependency('glib-2.0', version : glib_req, fallback: ['glib', 'libglib_dep']),
             dependency('gobject-2.0', fallback: ['glib', 'libgobject_dep'])]

json_dep = dependency('json-glib-1.0',
  fallback : ['json-glib', 'json_glib_dep']
)

# third party libraries
curl_dep = dependency('libcurl',
  version : '>= 7.55.0',
  required : true,
)

openssl_dep = dependency('openssl',
  version : '>= 1.0.0g',
  required : true
)

xml2_dep = dependency('libxml-2.0',
  version : '>= 2.8',
  fallback : ['libxml2', 'xml2lib_dep'],
  required: true
)


gst_cencdec_args = ['-DHAVE_CONFIG_H']
gst_test_args = ['-DHAVE_CONFIG_H']
configinc = include_directories('.')
libsinc = include_directories('gst-libs')

pkgconfig = import('pkgconfig')
plugins_pkgconfig_install_dir = join_paths(plugins_install_dir, 'pkgconfig')
if get_option('default_library') == 'shared'
  # If we don't build static plugins there is no need to generate pc files
  plugins_pkgconfig_install_dir = disabler()
endif

subdir('gst-libs')
subdir('gst')
subdir('tests')
subdir('pkgconfig')

configure_file(output : 'config.h', configuration : cdata)
