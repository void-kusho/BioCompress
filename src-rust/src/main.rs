/**
 * Biocompress - Rust CLI
 * Command-line interface for the compression library
 */
use clap::{Parser, Subcommand};
use std::path::PathBuf;
use thiserror::Error;

/// Custom error types for the CLI
#[derive(Error, Debug)]
pub enum BiocompressError {
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
    #[error("Compression error: {0}")]
    Compression(String),
    #[error("Invalid input: {0}")]
    InvalidInput(String),
}

/// Compression level options
#[derive(Parser, Debug)]
#[command(name = "biocompress")]
#[command(version = "0.1.0")]
#[command(about = "BioCompress - Compression tool for bioinformatics data")]
pub struct Cli {
    #[command(subcommand)]
    pub command: Commands,
}

#[derive(Subcommand, Debug)]
pub enum Commands {
    /// Compress a FASTA/FASTQ file
    Compress {
        /// Input file path
        #[arg(short, long)]
        input: PathBuf,

        /// Output file path
        #[arg(short, long)]
        output: PathBuf,

        /// Compression level (1=fastest, 5=best ratio)
        #[arg(short, long, default_value = "3")]
        level: u8,
    },

    /// Decompress a file
    Decompress {
        /// Input file path
        #[arg(short, long)]
        input: PathBuf,

        /// Output file path
        #[arg(short, long)]
        output: PathBuf,
    },

    /// Display version information
    Version,
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let cli = Cli::parse();

    match cli.command {
        Commands::Compress {
            input,
            output,
            level,
        } => {
            if level > 5 {
                return Err("Compression level must be between 1 and 5".into());
            }
            println!(
                "Compressing {} -> {} (level {})",
                input.display(),
                output.display(),
                level
            );
            // TODO: Implement actual compression
            Ok(())
        }

        Commands::Decompress { input, output } => {
            println!("Decompressing {} -> {}", input.display(), output.display());
            // TODO: Implement actual decompression
            Ok(())
        }

        Commands::Version => {
            println!("Biocompress {}", env!("CARGO_PKG_VERSION"));
            println!("C core version: {}", "0.1.0");
            Ok(())
        }
    }
}
