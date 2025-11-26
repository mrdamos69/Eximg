import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:image_editor_plus/image_editor_plus.dart';
import 'package:image_picker/image_picker.dart';

void main() {
  runApp(const EximgApp());
}

class EximgApp extends StatelessWidget {
  const EximgApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Eximg Photo Editor',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blueGrey),
        useMaterial3: true,
      ),
      home: const EditorHomePage(),
    );
  }
}

class EditorHomePage extends StatefulWidget {
  const EditorHomePage({super.key});

  @override
  State<EditorHomePage> createState() => _EditorHomePageState();
}

class _EditorHomePageState extends State<EditorHomePage> {
  File? _imageFile;
  bool _isEditing = false;

  Future<void> _pickImage(ImageSource source) async {
    final picker = ImagePicker();
    final pickedFile = await picker.pickImage(source: source, imageQuality: 100);
    if (pickedFile != null) {
      setState(() {
        _imageFile = File(pickedFile.path);
      });
    }
  }

  Future<void> _openEditor() async {
    final currentImage = _imageFile;
    if (currentImage == null || _isEditing) return;

    setState(() => _isEditing = true);
    try {
      final result = await Navigator.of(context).push(
        MaterialPageRoute<Uint8List?>(
          builder: (_) => ImageEditor(image: await currentImage.readAsBytes()),
        ),
      );
      if (result != null) {
        final editedFile = await _persistEditedImage(result);
        setState(() => _imageFile = editedFile);
      }
    } finally {
      if (mounted) setState(() => _isEditing = false);
    }
  }

  Future<File> _persistEditedImage(Uint8List bytes) async {
    final tempDir = await ImagePicker.platform.getTemporaryDirectory();
    final file = File('${tempDir.path}/eximg_edited_${DateTime.now().millisecondsSinceEpoch}.png');
    return file.writeAsBytes(bytes);
  }

  Widget _buildPlaceholder(BuildContext context) {
    return Container(
      height: 320,
      decoration: BoxDecoration(
        color: Theme.of(context).colorScheme.surfaceVariant,
        borderRadius: BorderRadius.circular(16),
      ),
      child: const Center(
        child: Text(
          'Выберите фото для редактирования',
          style: TextStyle(fontSize: 16),
        ),
      ),
    );
  }

  Widget _buildPreview() {
    final imageFile = _imageFile;
    if (imageFile == null) return _buildPlaceholder(context);
    return ClipRRect(
      borderRadius: BorderRadius.circular(16),
      child: Stack(
        children: [
          Positioned.fill(
            child: Image.file(
              imageFile,
              fit: BoxFit.cover,
            ),
          ),
          if (_isEditing)
            Positioned.fill(
              child: Container(
                color: Colors.black.withOpacity(0.35),
                child: const Center(
                  child: CircularProgressIndicator(),
                ),
              ),
            ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Eximg – редактор фото'),
        actions: [
          IconButton(
            onPressed: _imageFile != null && !_isEditing ? _openEditor : null,
            icon: const Icon(Icons.edit),
            tooltip: 'Редактировать',
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            _buildPreview(),
            const SizedBox(height: 24),
            Wrap(
              spacing: 12,
              runSpacing: 12,
              children: [
                ElevatedButton.icon(
                  onPressed: !_isEditing ? () => _pickImage(ImageSource.gallery) : null,
                  icon: const Icon(Icons.photo_library_outlined),
                  label: const Text('Галерея'),
                ),
                ElevatedButton.icon(
                  onPressed: !_isEditing ? () => _pickImage(ImageSource.camera) : null,
                  icon: const Icon(Icons.camera_alt_outlined),
                  label: const Text('Камера'),
                ),
                ElevatedButton.icon(
                  onPressed: _imageFile != null && !_isEditing ? _openEditor : null,
                  icon: const Icon(Icons.tune),
                  label: const Text('Открыть редактор'),
                ),
              ],
            ),
            const SizedBox(height: 16),
            const Text(
              'Редактор поддерживает обрезку, зеркалирование, вращение, наклейки, текст и фильтры',
              textAlign: TextAlign.center,
            ),
          ],
        ),
      ),
    );
  }
}
