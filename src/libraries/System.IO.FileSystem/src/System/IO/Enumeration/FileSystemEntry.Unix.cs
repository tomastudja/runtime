// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Diagnostics;

namespace System.IO.Enumeration
{
    /// <summary>
    /// Lower level view of FileSystemInfo used for processing and filtering find results.
    /// </summary>
    public unsafe ref partial struct FileSystemEntry
   {
        internal Interop.Sys.DirectoryEntry _directoryEntry;
        private FileStatus _status;
        private Span<char> _pathBuffer;
        private ReadOnlySpan<char> _fullPath;
        private ReadOnlySpan<char> _fileName;
        private fixed char _fileNameBuffer[Interop.Sys.DirectoryEntry.NameBufferSize];
        private FileAttributes _initialAttributes;

        internal static FileAttributes Initialize(
            ref FileSystemEntry entry,
            Interop.Sys.DirectoryEntry directoryEntry,
            ReadOnlySpan<char> directory,
            ReadOnlySpan<char> rootDirectory,
            ReadOnlySpan<char> originalRootDirectory,
            Span<char> pathBuffer)
        {
            entry._directoryEntry = directoryEntry;
            entry.Directory = directory;
            entry.RootDirectory = rootDirectory;
            entry.OriginalRootDirectory = originalRootDirectory;
            entry._pathBuffer = pathBuffer;
            entry._fullPath = ReadOnlySpan<char>.Empty;
            entry._fileName = ReadOnlySpan<char>.Empty;

            // IMPORTANT: Attribute logic must match the logic in FileStatus

            bool isDirectory = false;
            bool isSymlink = false;
            if (directoryEntry.InodeType == Interop.Sys.NodeType.DT_DIR)
            {
                // We know it's a directory.
                isDirectory = true;
            }
            // Some operating systems don't have the inode type in the dirent structure,
            // so we use DT_UNKNOWN as a sentinel value. As such, check if the dirent is a
            // directory.
            else if ((directoryEntry.InodeType == Interop.Sys.NodeType.DT_LNK
                || directoryEntry.InodeType == Interop.Sys.NodeType.DT_UNKNOWN)
                && Interop.Sys.Stat(entry.FullPath, out Interop.Sys.FileStatus statInfo) >= 0)
            {
                // Symlink or unknown: Stat to it to see if we can resolve it to a directory.
                isDirectory = FileStatus.IsDirectory(statInfo);
            }

            // Same idea as the directory check, just repeated for (and tweaked due to the nature of) symlinks.
            int resultLStat = Interop.Sys.LStat(entry.FullPath, out Interop.Sys.FileStatus lstatInfo);

            bool isReadOnly = resultLStat >= 0 && FileStatus.IsReadOnly(lstatInfo);

            if (directoryEntry.InodeType == Interop.Sys.NodeType.DT_LNK)
            {
                isSymlink = true;
            }
            else if (resultLStat >= 0 && directoryEntry.InodeType == Interop.Sys.NodeType.DT_UNKNOWN)
            {
                isSymlink = FileStatus.IsSymLink(lstatInfo);
            }

            // If the filename starts with a period or has UF_HIDDEN flag set, it's hidden.
            bool isHidden = directoryEntry.Name[0] == '.' || (resultLStat >= 0 && FileStatus.IsHidden(lstatInfo));

            entry._status = default;
            FileStatus.Initialize(ref entry._status, isDirectory);

            FileAttributes attributes = FileStatus.GetAttributes(isReadOnly, isSymlink, isDirectory, isHidden);

            entry._initialAttributes = attributes;
            return attributes;
        }

        private ReadOnlySpan<char> FullPath
        {
            get
            {
                if (_fullPath.Length == 0)
                {
                    Debug.Assert(Directory.Length + FileName.Length < _pathBuffer.Length,
                        $"directory ({Directory.Length} chars) & name ({Directory.Length} chars) too long for buffer ({_pathBuffer.Length} chars)");
                    Path.TryJoin(Directory, FileName, _pathBuffer, out int charsWritten);
                    Debug.Assert(charsWritten > 0, "didn't write any chars to buffer");
                    _fullPath = _pathBuffer.Slice(0, charsWritten);
                }
                return _fullPath;
            }
        }

        public ReadOnlySpan<char> FileName
        {
            get
            {
                if (_directoryEntry.NameLength != 0 && _fileName.Length == 0)
                {
                    fixed (char* c = _fileNameBuffer)
                    {
                        Span<char> buffer = new Span<char>(c, Interop.Sys.DirectoryEntry.NameBufferSize);
                        _fileName = _directoryEntry.GetName(buffer);
                    }
                }

                return _fileName;
            }
        }

        /// <summary>
        /// The full path of the directory this entry resides in.
        /// </summary>
        public ReadOnlySpan<char> Directory { get; private set; }

        /// <summary>
        /// The full path of the root directory used for the enumeration.
        /// </summary>
        public ReadOnlySpan<char> RootDirectory { get; private set; }

        /// <summary>
        /// The root directory for the enumeration as specified in the constructor.
        /// </summary>
        public ReadOnlySpan<char> OriginalRootDirectory { get; private set; }

        // Windows never fails getting attributes, length, or time as that information comes back
        // with the native enumeration struct. As such we must not throw here.

        public FileAttributes Attributes
            // It would be hard to rationalize if the attributes change after our initial find.
            => _initialAttributes | (_status.IsReadOnly(FullPath, continueOnError: true) ? FileAttributes.ReadOnly : 0);

        public long Length => _status.GetLength(FullPath, continueOnError: true);
        public DateTimeOffset CreationTimeUtc => _status.GetCreationTime(FullPath, continueOnError: true);
        public DateTimeOffset LastAccessTimeUtc => _status.GetLastAccessTime(FullPath, continueOnError: true);
        public DateTimeOffset LastWriteTimeUtc => _status.GetLastWriteTime(FullPath, continueOnError: true);
        public bool IsDirectory => _status.InitiallyDirectory;
        /// <summary>
        /// Returns <see langword="true"/> if the file is hidden; <see langword="false" /> otherwise.
        /// In Linux and OSX, a file can be marked hidden if the filename is prepended with a dot.
        /// In Windows and OSX, a file can be hidden if the special hidden attribute is set. For example, via the <see cref="FileSystemInfo.Attributes" /> enum flag.
        /// </summary>
        public bool IsHidden => _directoryEntry.Name[0] == '.' || (_initialAttributes & FileAttributes.Hidden) != 0;

        public FileSystemInfo ToFileSystemInfo()
        {
            string fullPath = ToFullPath();
            return FileSystemInfo.Create(fullPath, new string(FileName), ref _status);
        }

        /// <summary>
        /// Returns the full path of the find result.
        /// </summary>
        public string ToFullPath() =>
            new string(FullPath);
    }
}
