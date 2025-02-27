#ifndef MMAP_FILE_H
#define MMAP_FILE_H

#include <fstream>
#include <stdexcept>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

namespace MemoryMapping
{
	class MappedRegion
	{
	private:
		char* data_;
		size_t size_;
#ifdef _WIN32
		HANDLE mapping_handle_;
#endif

	public:
		MappedRegion(const std::string& filename, size_t offset, size_t size,
#ifdef _WIN32
		             HANDLE file_handle
#else
		             int fd
#endif
		             )
		    : data_(nullptr), size_(0)
		{
			struct stat st;
			if (stat(filename.c_str(), &st) == -1)
			{
				throw std::runtime_error("Error getting file size: " + filename);
			}
			size_t file_size = st.st_size;
			size_t map_size = std::min(size, file_size - offset);

#ifdef _WIN32
			mapping_handle_ = CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
			if (mapping_handle_ == NULL)
			{
				throw std::runtime_error("Error creating file mapping: " + filename);
			}

			data_ = static_cast<char*>(MapViewOfFile(mapping_handle_, FILE_MAP_READ, 0, offset, map_size));
			if (data_ == NULL)
			{
				CloseHandle(mapping_handle_);
				throw std::runtime_error("Error mapping view of file: " + filename);
			}
#else
			data_ = static_cast<char*>(mmap(nullptr, map_size, PROT_READ, MAP_PRIVATE, fd, offset));
			if (data_ == MAP_FAILED)
			{
				throw std::runtime_error("Error mapping file: " + filename);
			}
#endif
			size_ = map_size;
		}

		~MappedRegion()
		{
			if (data_ != nullptr)
			{
#ifdef _WIN32
				UnmapViewOfFile(data_);
				CloseHandle(mapping_handle_);
#else
				munmap(data_, size_);
#endif
			}
		}

		char* data() const { return data_; }

		size_t size() const { return size_; }
	};

	class MappedFile
	{
	private:
		std::string filename_;
		size_t file_size_;
#ifdef _WIN32
		HANDLE file_handle_;
#else
		int fd_;
#endif

	public:
		MappedFile(const std::string& filename)
		    : filename_(filename)
#ifdef _WIN32
		      ,
		      file_handle_(INVALID_HANDLE_VALUE)
#else
		      ,
		      fd_(-1)
#endif
		{
			struct stat st;
			if (stat(filename.c_str(), &st) == -1)
			{
				throw std::runtime_error("Error getting file size: " + filename);
			}
			file_size_ = st.st_size;

#ifdef _WIN32
			file_handle_ = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			                          FILE_ATTRIBUTE_NORMAL, NULL);
			if (file_handle_ == INVALID_HANDLE_VALUE)
			{
				throw std::runtime_error("Error opening file: " + filename);
			}
#else
			fd_ = open(filename.c_str(), O_RDONLY);
			if (fd_ == -1)
			{
				throw std::runtime_error("Error opening file: " + filename);
			}
#endif
		}

		~MappedFile()
		{
#ifdef _WIN32
			if (file_handle_ != INVALID_HANDLE_VALUE)
			{
				CloseHandle(file_handle_);
			}
#else
			if (fd_ != -1)
			{
				close(fd_);
			}
#endif
		}

		MappedRegion map(size_t offset, size_t size)
		{
			return MappedRegion(filename_, offset, size,
#ifdef _WIN32
			                    file_handle_
#else
			                    fd_
#endif
			);
		}

		size_t file_size() const { return file_size_; }
	};

	class RawFile
	{
	private:
		std::string temp_filename;
		std::ofstream writer;
		std::mutex write_synchronization;
		std::unique_ptr<MappedFile> mapper;
		std::mutex read_next_chunk_synchronization;
		size_t offset;

	public:
		RawFile() : mapper(nullptr)
		{
			// Create a temporary file
			char filePath[PATH_MAX] = "rawFileXXXXXX";
			int fd = mkstemp(filePath);  // Generate a unique temporary filename
			if (fcntl(fd, F_GETPATH, filePath) == -1)
			{
				throw std::runtime_error("Error creating temporary filename");
			}
			temp_filename = filePath;

			writer.open(temp_filename, std::ios::binary | std::ios::app);  // Open in binary append mode
			if (!writer.is_open())
			{
				throw std::runtime_error("Error opening temporary file for writing: " + temp_filename);
			}

			mapper = std::make_unique<MappedFile>(temp_filename);
			offset = 0;
		}

		~RawFile()
		{
			writer.close();
			if (std::remove(temp_filename.c_str()) != 0)  // Remove the temporary file
			{
				// Warning: Could not delete temporary file: ", temp_filename
			}
		}

		virtual void write(std::string_view str)
		{
			std::lock_guard<std::mutex> lk(write_synchronization);
			writer << str;
			writer.flush();  // Explicitly flush after writing
		}

		MappedRegion read_next_chunk(size_t chunk_size = 1 * 1024 * 1024)  // 1 MB default chunk size
		{
			std::lock_guard<std::mutex> lk(read_next_chunk_synchronization);
			if (offset >= mapper->file_size())
			{
				throw std::out_of_range("Reaching out of file " + temp_filename + "' s range.");
			}
			auto data = mapper->map(offset, chunk_size);
			offset += data.size();
			return data;
		}
	};
}  // namespace MemoryMapping

#endif  // MMAP_FILE_H
