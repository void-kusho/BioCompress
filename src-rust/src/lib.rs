/**
 * Biocompress - Rust library
 * FFI bindings to the C core library
 */
use std::ffi::CString;
use std::os::raw::c_char;

/// FFI bindings to C core library
extern "C" {
    fn biocompress_compress_file(
        input_path: *const c_char,
        output_path: *const c_char,
        level: i32,
    ) -> i32;
    fn biocompress_decompress_file(input_path: *const c_char, output_path: *const c_char) -> i32;
    fn biocompress_error_str(error_code: i32) -> *const c_char;
}

/// Rust wrapper for compress_file
pub fn compress_file(input: &str, output: &str, level: i32) -> Result<(), String> {
    let input_c = CString::new(input).map_err(|e| e.to_string())?;
    let output_c = CString::new(output).map_err(|e| e.to_string())?;

    let ret = unsafe { biocompress_compress_file(input_c.as_ptr(), output_c.as_ptr(), level) };

    if ret == 0 {
        Ok(())
    } else {
        let err_ptr = unsafe { biocompress_error_str(ret) };
        if err_ptr.is_null() {
            Err(format!("Compression failed with error code: {}", ret))
        } else {
            let err_msg = unsafe { CString::from_raw(err_ptr as *mut c_char) };
            Err(err_msg.to_string_lossy().into_owned())
        }
    }
}

/// Rust wrapper for decompress_file
pub fn decompress_file(input: &str, output: &str) -> Result<(), String> {
    let input_c = CString::new(input).map_err(|e| e.to_string())?;
    let output_c = CString::new(output).map_err(|e| e.to_string())?;

    let ret = unsafe { biocompress_decompress_file(input_c.as_ptr(), output_c.as_ptr()) };

    if ret == 0 {
        Ok(())
    } else {
        let err_ptr = unsafe { biocompress_error_str(ret) };
        if err_ptr.is_null() {
            Err(format!("Decompression failed with error code: {}", ret))
        } else {
            let err_msg = unsafe { CString::from_raw(err_ptr as *mut c_char) };
            Err(err_msg.to_string_lossy().into_owned())
        }
    }
}
