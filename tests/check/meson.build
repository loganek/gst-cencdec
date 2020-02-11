test_defines = [
  '-UG_DISABLE_ASSERT',
  '-UG_DISABLE_CAST_CHECKS',
  '-DGST_CHECK_TEST_ENVIRONMENT_BEACON="GST_STATE_IGNORE_ELEMENTS"',
  '-DGST_TEST_FILES_PATH="' + meson.current_source_dir() + '/../files"',
  '-DTEST_PATH="' + meson.current_build_dir() + '/media"',
  '-DDASH_MPD_DATADIR=' + meson.current_source_dir() + '/elements/dash_mpd_data',
  '-DGST_USE_UNSTABLE_API',
]

test_decrypt_exe = executable('aesctr/test-decrypt',
  'aesctr/decrypt.c',
  include_directories : [configinc, libsinc],
  dependencies: glib_deps + [
    gst_dep, gstbase_dep,
    gstcheck_dep, gstcencdrm_dep,
  ],
)

test('test-decrypt', test_decrypt_exe)
#subdir('aesctr')
