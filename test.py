from flask import Flask, render_template, request, jsonify
import subprocess
import tempfile

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/run_cpp', methods=['POST'])
def run_cpp():
    # 获取前端传递的输入
    input_text = request.json.get('inputText', '')

    # 将输入写入临时文件
    with tempfile.NamedTemporaryFile(delete=False, mode='w', encoding='utf-8') as input_file:
        input_file.write(input_text)
        input_file_path = input_file.name
    
    output_file_path = input_file_path + '.out'
    # 调用C++程序并传递输入文件路径
    result = call_cpp_program(input_file_path, output_file_path)
    if result is None:
        return jsonify({'error': 'C++ program error'}), 500
    
    # 读取C++程序的输出结果
    with open(output_file_path, 'r', encoding='utf-8') as output_file:
        output_text = output_file.read()

    # 返回处理后的数据给前端
    return jsonify({'outputText': output_text})

def call_cpp_program(input_file_path, output_file_path):
    # 调用C++程序并传递输入和输出文件路径
    process = subprocess.Popen(
        ['./ll1_parser', input_file_path, output_file_path], 
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    stdout, stderr = process.communicate()
    if stderr:
        print(f"Error: {stderr.decode('utf-8')}")
        return None
    
    return stdout.decode('utf-8')

if __name__ == '__main__':
    app.run(debug=True)